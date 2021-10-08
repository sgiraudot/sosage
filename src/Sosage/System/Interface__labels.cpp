/*
  [src/Sosage/System/Interface__labels.cpp]
  Handles labels, action selector, switchers, etc.

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Code.h>
#include <Sosage/Component/Group.h>
#include <Sosage/Component/GUI_animation.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Menu.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Interface.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/gamepad_labels.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

void Interface::create_object_label (const std::string& id)
{
  debug << "Create object_label " << id << std::endl;
  // Special case for inventory
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE))
  {
    auto position = get<C::Position>(id + ":position");
    auto pos = set<C::Relative_position>(id + "_label:global_position", position,
                                         Vector(0, -Config::inventory_height / 2 - 2 * Config::inventory_margin));

    const std::string& name = value<C::String>(id + ":name");
    create_label (false, id + "_label", locale(name), false, false, UNCLICKABLE);
    update_label(id + "_label", false, false, pos);

    double diff = value<C::Position>(id + "_label_back:position").x()
                  - 0.5 * get<C::Image>(id + "_label_back:image")->width()
                  - (value<C::Position>("Chamfer:position").x() + Config::label_height);
    if (diff < 0)
      position->set(Point (position->value().x() - diff, position->value().y()));

    animate_label (id + "_label", FADE);
    return;
  }

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (mode == MOUSE)
  {
    bool force_right = false;
    if (!request<C::String>("Interface:source_object"))
      force_right = value<C::Boolean>(id + "_goto:right", false);

    bool open_left = true;
    bool open_right = false;
    bool arrow = false;

    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      open_left = !right->value();
      open_right = right->value();
      arrow = true;
    }

    const std::string& name = value<C::String>(id + ":name");
    create_label (false, id + "_label", locale(name), open_left, open_right, UNCLICKABLE, 1.0, arrow);

    auto cursor = get<C::Position>(CURSOR__POSITION);
    update_label(id + "_label", open_left, open_right, cursor, 1.0, arrow);
    if (force_right || value<C::Position>(id + "_label_back:position").x()
        + get<C::Image>(id + "_label_back:image")->width() / 2
        > Config::world_width - Config::label_height)
    {
      open_left = false;
      open_right = true;
      create_label (false, id + "_label", locale(name), open_left, open_right, UNCLICKABLE, 1.0, arrow);
      update_label(id + "_label", open_left, open_right, cursor, 1.0, arrow);
    }
  }
  else
  {
    bool open_left = false;
    bool open_right = false;

    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      open_left = !right->value();
      open_right = right->value();
    }

    bool is_active = (mode == TOUCHSCREEN) || (m_active_object == id);
    double scale = (is_active ? 1.0 : 0.75);

    const std::string& name = value<C::String>(id + ":name");
    create_label (false, id + "_label", locale(name), open_left, open_right,
                  (mode == TOUCHSCREEN) ? BOX : UNCLICKABLE, scale, true);

    auto pos = set<C::Relative_position>
               (id + "_label:global_position",
                get<C::Position>(CAMERA__POSITION),
                value<C::Position>(id + ":label"), -1.);

    update_label(id + "_label", open_left, open_right, pos, scale, true);
  }

  animate_label (id + "_label", FADE);
}

void Interface::create_label (bool is_button, const std::string& id, std::string name,
                              bool open_left, bool open_right,
                              const Collision_type& collision, double scale, bool arrow)
{
  SOSAGE_TIMER_START(Interface__create_label);
  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  auto group = set<C::Group>(id + ":group");
  int depth = (is_button ? Config::action_button_depth : Config::label_depth);
  if (mode == GAMEPAD)
    depth = (is_button ? Config::menu_text_depth : Config::menu_front_depth);

  unsigned char alpha = (is_button ? 255 : 100);

  C::Image_handle label, left, right, back;
  if (name != "")
  {
    name[0] = toupper(name[0]);
    label = set<C::Image>(id + ":image", get<C::Font>("Interface:font"), "FFFFFF", name);
    group->add(label);

    label->set_relative_origin(0.5, 0.5);
    label->set_scale(scale * 0.5);
    label->z() = depth;
    label->set_collision(collision);
    label->set_alpha(scale * 255);
  }

  if (open_left)
  {
    if (arrow && mode != MOUSE)
      left = C::make_handle<C::Image>(id + "_left_circle:image", get<C::Image>("Goto_left:image"));
  }
  else
  {
    left = C::make_handle<C::Image>(id + "_left_circle:image", get<C::Image>("Left_circle:image"));
    left->set_alpha(alpha);
  }

  if (open_right)
  {
    if (arrow && mode != MOUSE)
      right = C::make_handle<C::Image>(id + "_right_circle:image", get<C::Image>("Goto_right:image"));
  }
  else
  {
    right = C::make_handle<C::Image>(id + "_right_circle:image", get<C::Image>("Right_circle:image"));
    right->set_alpha(alpha);
  }

  if (label)
  {
    int margin = Config::label_margin;
    if (request<C::String>("Interface:source_object"))
      margin *= 2;
    else if (open_left && open_right)
      margin *= 3;
    else if (!arrow && (open_left || open_right))
      margin *= 2;

    int width = margin + label->width() / 2;
    if (is_button || name.size() == 1)
      width = (name.size() - 1) * Config::label_margin;
    if (scale != 1.0)
    {
      // Avoid artefacts with bad divisions
      int factor = round(1. / (1. - scale));
      while (width % factor != 0)
        ++ width;
    }

    if (width != 0)
    {
      back = C::make_handle<C::Image>(id + "_back:image", width, Config::label_height);

      back->set_relative_origin(0.5, 0.5);
      back->set_scale(scale);
      back->set_alpha(alpha);
      back->z() = depth - 1;
      back->set_collision(collision);
    }
  }

  if (left)
  {
    if (back)
      left->compose_with (back);
    back = left;
  }
  if (right)
  {
    back->compose_with (right);
  }

  back->on() = true;
  back->set_relative_origin(0.5, 0.5);
  back->set_scale(scale);
  back->set_alpha(alpha);
  back->z() = depth - 1;
  back->set_collision(collision);

  back = set<C::Image>(id + "_back:image", back);
  group->add(back);

  SOSAGE_TIMER_STOP(Interface__create_label);
}

void Interface::animate_label (const std::string& id, const Animation_style& style,
                               bool button , const Point& position)
{
  debug << "Animate label " << id << std::endl;
  if (style == NONE)
    return;

  double current_time = value<C::Double>(CLOCK__TIME);

  if (!button)
  {
    if (style == ZOOM)
    {
      double from = 0.5 * 0.75, to = 0.5;
      double from_back = 0.75, to_back = 1.0;
      if (get<C::Image>(id + ":image")->scale() != 0.5)
      {
        std::swap(from, to);
        std::swap(from_back, to_back);
      }

      unsigned char alpha_back = get<C::Image>(id + "_back:image")->alpha();
      set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + ":image"), from, to, 255, 255);
      set<C::GUI_image_animation>(id + "_back:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back:image"), from_back, to_back,
                                  alpha_back, alpha_back);
    }
    else
    {
      auto img = get<C::Image>(id + "_back:image");
      set<C::GUI_image_animation>(id + "_back:animation", current_time, current_time + Config::inventory_speed,
                                  img, img->scale(), img->scale(), 0, img->alpha());

      if (style == DEPLOY)
        set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id + ":image"), 0.05, 0.5 * img->scale(), 0, 255);
      else if (style == FADE || style == FADE_LABEL_ONLY)
        set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id + ":image"), 0.5 * img->scale(), 0.5 * img->scale(), 0, 255);
    }
  }
  else
  {
    unsigned char alpha = get<C::Image>(id + "_back:image")->alpha();
    if (style == DEPLOY)
    {
      set<C::GUI_position_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                     get<C::Position>(id + ":global_position"), position);
      set<C::GUI_image_animation>(id + "_back:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back:image"), 0.357, 1, alpha, alpha);
    }
    else if (style == FADE)
    {
      set<C::GUI_image_animation>(id + "_back:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back:image"), 1, 1, 0, alpha);
    }
  }
}

void Interface::update_label (const std::string& id,
                              bool open_left, bool open_right, C::Position_handle pos,
                              double scale, bool arrow)
{
  auto label = request<C::Image>(id + ":image");
  auto back = get<C::Image>(id + "_back:image");
  int back_width = round (scale * back->width());

  // If animation was happening, finalize it before updating so that scale is final
  for (const std::string& section : { "", "_back" })
    if (auto anim = request<C::GUI_animation>(id + section + ":animation"))
    {
      debug << "Finalize " << anim->id() << std::endl;
      anim->finalize();
      remove(anim->id());
    }

  if (label)
    label->set_scale(scale * 0.5);
  back->set_scale(scale);

  if(open_left == open_right) // symmetric label
  {
    set<C::Relative_position>(id + ":position", pos);
    set<C::Relative_position>(id + "_back:position", pos);
  }
  else if (open_left)
  {
    if (label)
    {
      double label_pos = back_width - scale * Config::label_margin - round (label->scale() * label->width()) * 0.5;
      set<C::Relative_position>(id + ":position", pos, Vector(label_pos, 0));
    }
    set<C::Relative_position>(id + "_back:position", pos, Vector(back_width / 2, 0));
  }
  else if (open_right)
  {
    if (label)
    {
      double label_pos = - back_width + scale * Config::label_margin + round (label->scale() * label->width()) * 0.5;
      set<C::Relative_position>(id + ":position", pos, Vector(label_pos, 0));
    }
    set<C::Relative_position>(id + "_back:position", pos, Vector(-back_width / 2, 0));
  }
}

void Interface::update_label_position (const std::string& id, double scale)
{
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE))
    return;
  debug << "Update label position " << id << std::endl;

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (mode == MOUSE)
  {
    bool force_right = false;
    if (!request<C::String>("Interface:source_object"))
      force_right = value<C::Boolean>(id + "_goto:right", false);

    auto cursor = get<C::Position>(CURSOR__POSITION);
    update_label(id + "_label", true, false, cursor, scale);
    if (force_right || value<C::Position>(id + "_label_back:position").x() >
        Config::world_width - Config::label_height)
      update_label(id + "_label", false, true, cursor, scale);
  }
  else
  {
    bool open_left = false;
    bool open_right = false;
    bool arrow = false;

    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      open_left = !right->value();
      open_right = right->value();
      arrow = true;
    }

    auto pos = set<C::Relative_position>
               (id + "_label:global_position",
                get<C::Position>(CAMERA__POSITION),
                value<C::Position>(id + ":label"), -1.);

    update_label(id + "_label", open_left, open_right, pos, scale, arrow);
  }
}

void Interface::delete_label (const std::string& id)
{
  auto group = request<C::Group>(id + ":group");
  if (!group)
    return;
  debug << "Delete label " << id << std::endl;

  remove(id + ":global_position", true);
  double current_time = value<C::Double>(CLOCK__TIME);
  group->apply<C::Image>([&](auto img_old)
  {
    // To avoid later referencing a fading-out image, copy it with another ID and animate the copy
    auto img = set<C::Image>(img_old->entity() + "_old:image", img_old);
    set<C::Variable>(img_old->entity() + "_old:position", get<C::Position>(img_old->entity() + ":position"));
    remove(img_old->id());
    remove(img_old->entity() + ":position");
    set<C::GUI_image_animation>(img->entity() + ":animation", current_time, current_time + Config::inventory_speed,
                                img, img->scale(), img->scale(), img->alpha(), 0, true);
  });
  remove(group->id());
}

void Interface::fade_action_selector (const std::string& id, bool fade_in)
{
  // Exit if animation already happening
  if (request<C::GUI_image_animation>(id + "_left:animation"))
    return;
  double current_time = value<C::Double>(CLOCK__TIME);
  auto group = get<C::Group>(id + ":group");
  group->apply<C::Image>([&](auto img)
  {
    unsigned char alpha_off = 0;
    unsigned char alpha_on = 255;
    if (contains(img->id(), "_label") && !contains(img->id(), "_label:"))
      alpha_on = 100;

    img->on() = true;
    set<C::GUI_image_animation>(img->entity() + ":animation", current_time, current_time + Config::inventory_speed,
                                img, img->scale(), img->scale(),
                                (fade_in ? alpha_off : alpha_on),
                                (fade_in ? alpha_on : alpha_off),
                                false);
  });
}

void Interface::highlight_object (const std::string& id, unsigned char highlight)
{
  debug << "Highlight " << id << " by " << highlight << std::endl;
  // Image might have been destroyed here
  if (auto img = request<C::Image>(id + ":image"))
    img->set_highlight (highlight);
}

void Interface::set_action_selector (const std::string& id)
{
  SOSAGE_TIMER_START(Interface__set_action_selector);
  debug << "Set action selector to " << id << std::endl;
  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  if (mode == GAMEPAD)
  {
    std::string take_id = id;
    std::string look_id = id;
    std::string move_id = id;
    std::string inventory_id = (id == "" ? "Default" : id);
    std::string take_action = "take";
    std::string look_action = "look";
    std::string move_action = "move";
    std::string inventory_action = "inventory";
    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      take_id = "";
      move_id = "";
      inventory_id = "Default";
      look_action = "goto";
    }

    if (status()->is (IN_INVENTORY))
    {
      take_action = "combine";
      move_action = "use";
      inventory_action = "Cancel";
      if (get<C::Inventory>("Game:inventory")->size() == 1)
        take_id = "";
    }
    else if (status()->is (OBJECT_CHOICE))
    {
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Cancel";
    }
    else if (status()->is (IN_CODE, IN_WINDOW))
    {
      look_id = "code";
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Cancel";
    }
    else if (status()->is (IN_MENU))
    {
      look_id = "menu";
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Continue";
    }

    const Gamepad_type& gamepad = value<C::Simple<Gamepad_type>>("Gamepad:type");
    auto pos = get<C::Position>("Gamepad_action_selector:position");
    generate_action (take_id, take_action, LEFT_BUTTON, gamepad_label(gamepad, WEST), pos, FADE_LABEL_ONLY);
    generate_action (look_id, look_action, RIGHT_BUTTON, gamepad_label(gamepad, EAST), pos, FADE_LABEL_ONLY);
    generate_action (move_id, move_action, UP, gamepad_label(gamepad, NORTH), pos, FADE_LABEL_ONLY);
    generate_action (inventory_id, inventory_action, DOWN, gamepad_label(gamepad, SOUTH), pos, FADE_LABEL_ONLY);
  }
  else
  {
    if (status()->is (INVENTORY_ACTION_CHOICE))
    {
      Point object_pos = value<C::Position>(id + ":position");
      auto position = set<C::Absolute_position>
                      ("Action_selector:position",
                       Point (object_pos.x(),
                              value<C::Position>("Inventory:origin").y() - 0.75 * Config::label_height));

      generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
      double diff = value<C::Position>(id + "_combine_label_back:position").x()
                    - 0.5 * get<C::Image>(id + "_combine_label_back:image")->width()
                    - (value<C::Position>("Chamfer:position").x() + Config::label_height);
      if (diff < 0)
      {
        position->set(Point (position->value().x() - diff, position->value().y()));
        generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
      }

      generate_action (id, "use", UP, "", position, DEPLOY);
      generate_action (id, "look", RIGHT_BUTTON, "", position, DEPLOY);
    }
    else
    {
      auto position = set<C::Absolute_position>("Action_selector:position",
                                                value<C::Position>(CURSOR__POSITION));


      generate_action (id, "take", LEFT_BUTTON, "", position, DEPLOY);
      if (value<C::Position>(id + "_take_label_back:position").x()
          - 0.5 * get<C::Image>(id + "_take_label_back:image")->width()
          < Config::label_height)
      {
        generate_action (id, "move", UPPER, "", position, DEPLOY);
        generate_action (id, "take", UP_RIGHT, "", position, DEPLOY);
        generate_action (id, "look", DOWN_RIGHT, "", position, DEPLOY);
        generate_action (id, "inventory", DOWNER, "", position, DEPLOY);
      }
      else
      {
        generate_action (id, "look", RIGHT_BUTTON, "", position, DEPLOY);
        if (value<C::Position>(id + "_take_label_back:position").x()
            + 0.5 * get<C::Image>(id + "_take_label_back:image")->width()
            > Config::world_width - Config::label_height)
        {
          generate_action (id, "move", UPPER, "", position, DEPLOY);
          generate_action (id, "take", UP_LEFT, "", position, DEPLOY);
          generate_action (id, "look", DOWN_LEFT, "", position, DEPLOY);
          generate_action (id, "inventory", DOWNER, "", position, DEPLOY);

        }
        else
        {
          generate_action (id, "move", UP, "", position, DEPLOY);
          generate_action (id, "inventory", DOWN, "", position, DEPLOY);
        }
      }
    }
  }
  SOSAGE_TIMER_STOP(Interface__set_action_selector);
}

void Interface::reset_action_selector()
{
  debug << "Reset action selector" << std::endl;
  for (std::string& id : m_action_selector)
  {
    delete_label (id + "_label");
    delete_label (id + "_button");
    id = "";
  }
}

void Interface::generate_action (const std::string& id, const std::string& action,
                                 const Button_orientation& orientation, const std::string& button,
                                 C::Position_handle position, const Animation_style& style)
{
  auto label = request<C::String>(id + "_" + action + ":label");
  if (isupper(action[0]))
    label = get<C::String>(action + ":text");
  if (!label)
    label = get<C::String>("Default_" + action + ":label");
  if (id == "Default" && action == "inventory")
    label = get<C::String>("Inventory:label");

  bool open_left = false, open_right = false;
  Vector label_position;
  Vector button_position;
  Vector start_position;

  // Default cross orientation
  std::size_t selector_idx = 0;
  if (orientation == UP)
  {
    selector_idx = 0;
    label_position = Vector(0, -80);
    button_position = Vector(0, -40);
    start_position = Vector(0, -14);
  }
  else if (orientation == RIGHT_BUTTON)
  {
    selector_idx = 1;
    label_position = Vector(40, 0);
    button_position = Vector(40, 0);
    start_position = Vector(14, 0);
    open_left = true;
  }
  else if (orientation == DOWN)
  {
    selector_idx = 2;
    label_position = Vector(0, 80);
    button_position = Vector(0, 40);
    start_position = Vector(0, 14);
  }
  else if (orientation == LEFT_BUTTON)
  {
    selector_idx = 3;
    label_position = Vector(-40, 0);
    button_position = Vector(-40, 0);
    start_position = Vector(-14, 0);
    open_right = true;
  }

  // Side orientations
  else if (orientation == UPPER)
  {
    selector_idx = 0;
    label_position = Vector(0, -100);
    button_position = Vector(0, -60);
    start_position = Vector(0, -14);
  }
  else if (orientation == DOWN_RIGHT)
  {
    selector_idx = 1;
    label_position = Vector(50, 28.25);
    button_position = Vector(50, 28.25);
    start_position = Vector(14, 0);
    open_left = true;
  }
  else if (orientation == DOWN_LEFT)
  {
    selector_idx = 1;
    label_position = Vector(-50, 28.25);
    button_position = Vector(-50, 28.25);
    start_position = Vector(14, 0);
    open_right = true;
  }
  else if (orientation == DOWNER)
  {
    selector_idx = 2;
    label_position = Vector(0, 100);
    button_position = Vector(0, 60);
    start_position = Vector(0, 14);
  }
  else if (orientation == UP_RIGHT)
  {
    selector_idx = 3;
    label_position = Vector(50, -28.25);
    button_position = Vector(50, -28.25);
    start_position = Vector(-14, 0);
    open_left = true;
  }
  else if (orientation == UP_LEFT)
  {
    selector_idx = 3;
    label_position = Vector(-50, -28.25);
    button_position = Vector(-50, -28.25);
    start_position = Vector(-14, 0);
    open_right = true;
  }
  m_action_selector[selector_idx] = id + "_" + action;

  if (style != DEPLOY)
    start_position = button_position;

  if (id != "")
  {
    std::string label_id = id + "_" + action + "_label";
    create_label (false, label_id, locale(label->value()), open_left, open_right, BOX);
    update_label (label_id, open_left, open_right,
                  set<C::Relative_position>(label_id + ":global_position", position, label_position));

    // UPPER and DOWNER configs might need to be moved to be on screen
    if (orientation == UPPER || orientation == DOWNER)
    {
      int diff = 0;
      auto pos = get<C::Position>(label_id + "_back:position");
      auto img =  get<C::Image>(label_id + "_back:image");
      if (pos->value().x() - 0.5 * img->width() < Config::label_margin)
        diff = Config::label_margin - pos->value().x() + 0.5 * img->width();
      else if (pos->value().x() + 0.5 * img->width() > Config::world_width - Config::label_margin)
        diff = pos->value().x() - 0.5 * img->width() - (Config::world_width - Config::label_margin);

      if (diff != 0)
      {
        auto pos = get<C::Position>(label_id + ":global_position");
        pos->set (Point (pos->value().x() + diff, pos->value().y()));
      }
    }

    animate_label (label_id, style);
  }

  std::string button_id = "";
  if (id == "")
  {
    m_action_selector[selector_idx] = "Default_" + action;
    button_id = "Default_" + action + "_button";
    create_label (true, button_id, button, false, false, BOX);
    update_label (button_id, false, false,
                  set<C::Relative_position>(button_id + ":global_position", position, start_position));
    if (auto img = request<C::Image>("Default_" + action + "_button:image"))
      img->on() = false;
    get<C::Image>("Default_" + action + "_button_back:image")->set_alpha(128);
  }
  else
  {
    button_id = id + "_" + action + "_button";
    create_label (true, button_id, button, false, false, BOX);
    update_label (button_id, false, false,
                  set<C::Relative_position>(button_id + ":global_position", position, start_position));
  }

  animate_label (button_id, style, true, position->value() + button_position);
}

} // namespace Sosage::System
