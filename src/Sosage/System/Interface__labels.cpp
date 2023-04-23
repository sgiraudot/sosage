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
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/gamepad_labels.h>
#include <Sosage/Utils/helpers.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

void Interface::create_object_label (const std::string& id)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::create_object_label()");

  debug << status()->str() << std::endl;
  debug << "Create object_label " << id << std::endl;
  // Special case for inventory
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE))
  {
    auto position = get<C::Position>(id , "position");
    auto pos = set<C::Relative_position>(id + "_label", "global_position", position,
                                         Vector(0, -Config::inventory_height / 2 - 2 * Config::inventory_margin * Config::interface_scale));

    const std::string& name = value<C::String>(id , "name");
    create_label (id + "_label", locale(name), PLAIN, UNCLICKABLE);
    update_label(id + "_label", PLAIN, pos);

    double diff = value<C::Position>(id + "_label_back", "position").x()
                  - 0.25 * get<C::Image>(id + "_label_back", "image")->width() * Config::interface_scale
                  - (value<C::Position>("Chamfer", "position").x() + Config::label_height);
    if (diff < 0)
      pos->set(Point (pos->value().x() - diff, pos->value().y()));

    diff = Config::world_width - (value<C::Position>(id + "_label_back", "position").x()
                                  + (0.25 * get<C::Image>(id + "_label_back", "image")->width() + Config::label_margin)
                                  * Config::interface_scale);

    if (diff < 0)
      pos->set(Point (pos->value().x() + diff, pos->value().y()));

    animate_label (id + "_label", FADE);
    return;
  }

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (mode == MOUSE)
  {
    bool force_right = false;
    if (!request<C::String>("Interface", "source_object"))
      force_right = value<C::Boolean>(id + "_goto", "right", false);

    Label_type ltype = CURSOR_LEFT;

    if (auto right = request<C::Boolean>(id + "_goto", "right"))
      ltype = right->value() ? GOTO_RIGHT : GOTO_LEFT;

    const std::string& name = value<C::String>(id , "name");
    create_label (id + "_label", locale(name), ltype, UNCLICKABLE, 1.0);

    auto cursor = get<C::Position>(CURSOR__POSITION);
    update_label(id + "_label", ltype, cursor, 1.0);
    if (force_right || value<C::Position>(id + "_label_back", "position").x()
        + get<C::Image>(id + "_label_back", "image")->width() * 0.25
        > Config::world_width - Config::label_height)
    {
      ltype = force_right ? GOTO_RIGHT : CURSOR_RIGHT;
      create_label (id + "_label", locale(name), ltype, UNCLICKABLE, 1.0);
      update_label(id + "_label", ltype, cursor, 1.0);
    }
  }
  else
  {
    Label_type ltype = PLAIN;

    if (auto right = request<C::Boolean>(id + "_goto", "right"))
      ltype = right->value() ? GOTO_RIGHT : GOTO_LEFT;

    bool is_active = (mode == TOUCHSCREEN) || (m_active_object == id);
    double scale = (is_active ? 1.0 : 0.75);

    const std::string& name = value<C::String>(id , "name");
    create_label (id + "_label", locale(name), ltype,
                  (mode == TOUCHSCREEN) ? BOX : UNCLICKABLE, scale);
    if (!is_active)
      get<C::Image>(id + "_label", "image")->set_alpha(192);

    auto base_pos = set<C::Double_relative_position>
                    (id + "_label", "base_global_position",
                     get<C::Position>(CAMERA__POSITION),
                     get<C::Position>(id , "label"), -1.);
    auto pos = wriggly_position (id + "_label", "global_position", base_pos, Vector(),
                                 (ltype != PLAIN) ? RIGHT_BUTTON : UP, true, true);

    update_label(id + "_label", ltype, pos, scale);
  }

  animate_label (id + "_label", FADE);
}

void Interface::create_label (const std::string& id, std::string name,
                              const Label_type& ltype,
                              const Collision_type& collision, double scale)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::create_label()");

  SOSAGE_TIMER_START(Interface__create_label);
  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  double scaled_alpha = scale * 255;
  scale *= Config::interface_scale;

  auto group = set<C::Group>(id , "group");
  int depth = (ltype == LABEL_BUTTON ? Config::action_button_depth : Config::label_depth);
  if (mode == GAMEPAD)
    depth = (ltype == LABEL_BUTTON ? Config::menu_text_depth : Config::menu_front_depth);

  unsigned char alpha = (ltype == LABEL_BUTTON ? 255 : 100);

  C::Image_handle label, left, right, back;
  if (name != "")
  {
    capitalize(name);
    label = set<C::Image>(id , "image", get<C::Font>("Interface", "font"), "FFFFFF", name);
    group->add(label);

    label->set_relative_origin(0.5, 0.5);
    label->set_scale(scale * 0.5);
    label->z() = depth;
    label->set_collision(collision);
    label->set_alpha(scaled_alpha);
  }

  if (ltype == GOTO_LEFT && mode != MOUSE)
      left = C::make_handle<C::Image>(id + "_left_circle", "image", get<C::Image>("Goto_left", "image"));

  if (ltype != GOTO_LEFT && ltype != OPEN && ltype != CURSOR_LEFT)
  {
    left = C::make_handle<C::Image>(id + "_left_circle", "image", get<C::Image>("Left_circle", "image"));
    left->set_alpha(alpha);
  }

  if (ltype == GOTO_RIGHT && mode != MOUSE)
      right = C::make_handle<C::Image>(id + "_right_circle", "image", get<C::Image>("Goto_right", "image"));

  if (ltype != GOTO_RIGHT && ltype != OPEN && ltype != CURSOR_RIGHT)
  {
    right = C::make_handle<C::Image>(id + "_right_circle", "image", get<C::Image>("Right_circle", "image"));
    right->set_alpha(alpha);
  }

  if (label)
  {
    int margin = Config::label_margin;
    if (ltype == CURSOR_LEFT || ltype == CURSOR_RIGHT)
      margin = 2 * Config::label_margin;
    else if (ltype == OPEN)
      margin = 3 * Config::label_margin;
    else if (ltype == OPEN_LEFT || ltype == OPEN_RIGHT)
      margin = 2 * Config::label_margin;
    else if (ltype == GOTO_LEFT || ltype == GOTO_RIGHT)
      margin = Config::label_margin;

    if (request<C::String>("Interface", "source_object"))
      margin = 3 * Config::label_margin;

    int width = margin + label->width() / 2;
    if (ltype == LABEL_BUTTON || name.size() == 1)
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
      back = C::make_handle<C::Image>(id + "_back", "image", 2* width, 2 * Config::label_height);

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
  back->set_scale(0.5 * scale);
  back->set_alpha(alpha);
  back->z() = depth - 1;
  back->set_collision(collision);

  back = set<C::Image>(id + "_back", "image", back);
  group->add(back);

  SOSAGE_TIMER_STOP(Interface__create_label);
}

void Interface::animate_label (const std::string& id, const Animation_style& style,
                               bool button , const Point& position)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::animate_label()");

  debug << "Animate label " << id << std::endl;
  if (style == NONE)
    return;

  double current_time = value<C::Double>(CLOCK__TIME);

  if (!button)
  {
    if (style == ZOOM)
    {
      double from = 0.5 * 0.75, to = 0.5;
      double from_back = 0.75 * 0.5, to_back = 0.5;
      from *= Config::interface_scale;
      from_back *= Config::interface_scale;
      to *= Config::interface_scale;
      to_back *= Config::interface_scale;

      unsigned char alpha_label_from = 192, alpha_label_to = 255;
      if (get<C::Image>(id , "image")->scale() != 0.5 * Config::interface_scale)
      {
        std::swap(from, to);
        std::swap(from_back, to_back);
        std::swap(alpha_label_from, alpha_label_to);
      }

      unsigned char alpha_back = get<C::Image>(id + "_back", "image")->alpha();
      set<C::GUI_image_animation>(id , "animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id , "image"), from, to, alpha_label_from, alpha_label_to);
      set<C::GUI_image_animation>(id + "_back", "animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back", "image"), from_back, to_back,
                                  alpha_back, alpha_back);

      // If label not centered, position must change a bit
      std::size_t pos = id.find("_label");
      if (pos != std::string::npos)
      {
        std::string object_id (id.begin(), id.begin() + pos);
        if (auto right = request<C::Boolean>(object_id + "_goto", "right"))
        {
          double gap = get<C::Image>(id + "_back", "image")->width() * 0.25;
          if (!right->value())
            gap = -gap;
          double gap_from = from_back * gap;
          double gap_to = to_back * gap;

          auto pos = get<C::Relative_position>(id + "_back", "position");
          auto pos_to = pos->value();
          auto pos_from = pos_to - Vector(gap_from, 0) + Vector(gap_to, 0);
          pos->set(pos_from);
          set<C::GUI_position_animation>(id + "_back_pos", "animation", current_time, current_time + Config::inventory_speed,
                                         pos, pos_to);

          double label_gap = get<C::Image>(id , "image")->width() * 0.5;
          if (!right->value())
            label_gap = -label_gap;

          double label_from = 2*gap_from - from * Config::label_margin - round (from * label_gap);
          double label_to = 2*gap_to - to * Config::label_margin - round (to * label_gap);

          auto lpos = get<C::Relative_position>(id , "position");
          auto lpos_to = lpos->value();
          auto lpos_from = lpos_to - Vector(label_from, 0) + Vector(label_to, 0);
          lpos->set(lpos_from);
          set<C::GUI_position_animation>(id + "_pos", "animation", current_time, current_time + Config::inventory_speed,
                                         lpos, lpos_to);
        }
      }
    }
    else
    {
      auto img = get<C::Image>(id + "_back", "image");
      set<C::GUI_image_animation>(id + "_back", "animation", current_time, current_time + Config::inventory_speed,
                                  img, img->scale(), img->scale(), 0, img->alpha());

      if (style == DEPLOY)
        set<C::GUI_image_animation>(id , "animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id , "image"), 0.05, img->scale(), 0, 255);
      else if (style == FADE || style == FADE_LABEL_ONLY)
      {
        unsigned char alpha = get<C::Image>(id, "image")->alpha();
        set<C::GUI_image_animation>(id , "animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id , "image"), img->scale(), img->scale(), 0, alpha);
      }
    }
  }
  else
  {
    unsigned char alpha = get<C::Image>(id + "_back", "image")->alpha();
    if (style == DEPLOY)
    {
      set<C::GUI_position_animation>(id , "animation", current_time, current_time + Config::inventory_speed,
                                     get<C::Position>(id , "global_position"), position);
      set<C::GUI_image_animation>(id + "_back", "animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back", "image"), 0.177 * Config::interface_scale,
                                  0.5 * Config::interface_scale, alpha, alpha);
    }
    else if (style == FADE)
    {
      set<C::GUI_image_animation>(id + "_back", "animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back", "image"), 0.5, 0.5, 0, alpha);
    }
  }
}

void Interface::update_label (const std::string& id, const Label_type& ltype,
                              C::Position_handle pos, double scale)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_label()");

  scale *= Config::interface_scale;

  auto label = request<C::Image>(id , "image");
  auto back = get<C::Image>(id + "_back", "image");
  int back_width = round (scale * back->width());
  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  // If animation was happening, finalize it before updating so that scale is final
  for (const std::string& section : { "", "_back" })
    if (auto anim = request<C::GUI_animation>(id + section , "animation"))
    {
      debug << "Finalize " << anim->str() << std::endl;
      anim->finalize();
      remove(anim);
    }

  if (label)
    label->set_scale(scale * 0.5);
  back->set_scale(scale * 0.5);

  if(ltype == PLAIN || ltype == OPEN || ltype == LABEL_BUTTON) // symmetric label
  {
    set<C::Relative_position>(id , "position", pos);
    set<C::Relative_position>(id + "_back", "position", pos);
  }
  else if (ltype == OPEN_LEFT || ltype == GOTO_LEFT || ltype == CURSOR_LEFT)
  {
    if (label)
    {
      double label_pos = 0.25 * back_width;
      if (ltype == CURSOR_LEFT)
        label_pos += 5;
      else if (ltype == OPEN_LEFT)
        label_pos -= 5;
      else if (ltype == GOTO_LEFT && mode != MOUSE)
        label_pos += 30 * scale;
      set<C::Relative_position>(id , "position", pos, Vector(label_pos, 0));
    }
    if (ltype == OPEN_LEFT)
      set<C::Relative_position>(id + "_back", "position", pos, Vector(back_width * 0.25 - 25 * Config::interface_scale, 0));
    else
      set<C::Relative_position>(id + "_back", "position", pos, Vector(back_width * 0.25, 0));
  }
  else if (ltype == OPEN_RIGHT || ltype == GOTO_RIGHT || ltype == CURSOR_RIGHT)
  {
    if (label)
    {
      double label_pos = -0.25 * back_width;
      if (ltype == CURSOR_RIGHT)
        label_pos -= 5;
      else if (ltype == OPEN_RIGHT)
        label_pos += 5;
      else if (ltype == GOTO_RIGHT && mode != MOUSE)
        label_pos -= 30 * scale;

      set<C::Relative_position>(id , "position", pos, Vector(label_pos, 0));
    }
    if (ltype == OPEN_RIGHT)
      set<C::Relative_position>(id + "_back", "position", pos, Vector(-back_width * 0.25 + 25 * Config::interface_scale, 0));
    else
      set<C::Relative_position>(id + "_back", "position", pos, Vector(-back_width * 0.25, 0));
  }
}

void Interface::update_label_position (const std::string& id, double scale)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_label_position()");

  if (status()->is (IN_INVENTORY, OBJECT_CHOICE))
    return;
  debug << "Update label position " << id << std::endl;

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (mode == MOUSE)
  {
    bool force_right = false;
    if (!request<C::String>("Interface", "source_object"))
      force_right = value<C::Boolean>(id + "_goto", "right", false);

    auto cursor = get<C::Position>(CURSOR__POSITION);
    update_label(id + "_label", OPEN_LEFT, cursor, scale);
    if (force_right || value<C::Position>(id + "_label_back", "position").x() >
        Config::world_width - Config::label_height)
      update_label(id + "_label", OPEN_RIGHT, cursor, scale);
  }
  else
  {
    Label_type ltype = PLAIN;

    if (auto right = request<C::Boolean>(id + "_goto", "right"))
      ltype = right->value() ? GOTO_RIGHT : GOTO_LEFT;

    auto base_pos = set<C::Relative_position>
                    (id + "_label", "base_global_position",
                     get<C::Position>(CAMERA__POSITION),
                     value<C::Position>(id , "label"), -1.);
    auto pos = wriggly_position (id + "_label", "global_position", base_pos, Vector(),
                                 (ltype != PLAIN) ? RIGHT_BUTTON : UP, true, true);

    update_label(id + "_label", ltype, pos, scale);
  }
}

void Interface::delete_label (const std::string& id, bool animate)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::delete_label()");

  auto group = request<C::Group>(id , "group");
  if (!group)
  {
    debug << "Can' delete " << id << std::endl;
    return;
  }
  debug << "Delete label " << id << std::endl;

  remove(id , "global_position", true);
  if (animate)
  {
    double current_time = value<C::Double>(CLOCK__TIME);
    group->apply<C::Image>([&](auto img_old)
    {
      // To avoid later referencing a fading-out image, copy it with another ID and animate the copy
      auto img = set<C::Image>(img_old->entity() + "_old", "image", img_old);
      set<C::Variable>(img_old->entity() + "_old", "position", get<C::Position>(img_old->entity() , "position"));
      remove(img_old->entity() , "position");
      remove(img_old);
      set<C::GUI_image_animation>(img->entity() , "animation", current_time, current_time + Config::inventory_speed,
                                  img, img->scale(), img->scale(), img->alpha(), 0, true);
    });
  }
  else
  {
    group->apply<C::Image>([&](auto img)
    {
      remove (img);
    });
  }
  remove(group);
}

void Interface::fade_action_selector (const std::string& id, bool fade_in)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::fade_action_selector()");

  // Exit if animation already happening
  if (request<C::GUI_image_animation>(id + "_left", "animation"))
    return;
  double current_time = value<C::Double>(CLOCK__TIME);
  auto group = get<C::Group>(id , "group");
  group->apply<C::Image>([&](auto img)
  {
    unsigned char alpha_off = 0;
    unsigned char alpha_on = 255;
    debug << img->entity() << std::endl;
    if (contains(img->entity(), "_label") && !endswith(img->entity(), "_label"))
      alpha_on = 100;

    img->on() = true;
    set<C::GUI_image_animation>(img->entity() , "animation", current_time, current_time + Config::inventory_speed,
                                img, img->scale(), img->scale(),
                                (fade_in ? alpha_off : alpha_on),
                                (fade_in ? alpha_on : alpha_off),
                                false);
  });
}

void Interface::highlight_object (const std::string& id, unsigned char highlight)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::highlight_object()");

  debug << "Highlight " << id << " by " << int(highlight) << std::endl;
  // Image might have been destroyed here
  if (auto img = request<C::Image>(id , "image"))
    img->set_highlight (highlight);
}

void Interface::set_action_selector (const Selector_type& type, const std::string& id)
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::set_action_selector()");

  // Already up to date, do nothing
  if (m_selector_type == type && m_selector_id == id)
    return;

  SOSAGE_TIMER_START(Interface__set_action_selector);

  debug << "Set " << type << " action selector to " << id << std::endl;

  reset_action_selector();

  m_selector_type = type;
  m_selector_id = id;

  const Gamepad_type& gamepad = value<C::Simple<Gamepad_type>>("Gamepad", "type");
  auto gamepad_pos = get<C::Position>("Gamepad_action_selector", "position");
  if (type == GP_IDLE)
  {
    generate_action ("", "take", LEFT_BUTTON, gamepad_label(gamepad, WEST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action (id, (id == "" ? "look" : "Action"), RIGHT_BUTTON,
                     gamepad_label(gamepad, EAST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("", "move", UP, gamepad_label(gamepad, NORTH), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("Default", "inventory", DOWN, gamepad_label(gamepad, SOUTH), gamepad_pos, FADE_LABEL_ONLY);
  }
  else if (type == OKNOTOK)
  {
    generate_action ("", "take", LEFT_BUTTON, gamepad_label(gamepad, WEST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("oknotok", "Ok", RIGHT_BUTTON, gamepad_label(gamepad, EAST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("", "move", UP, gamepad_label(gamepad, NORTH), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("oknotok", "Cancel", DOWN, gamepad_label(gamepad, SOUTH), gamepad_pos, FADE_LABEL_ONLY);
  }
  else if (type == OKCONTINUE)
  {
    generate_action ("", "take", LEFT_BUTTON, gamepad_label(gamepad, WEST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("okcontinue", "Ok", RIGHT_BUTTON, gamepad_label(gamepad, EAST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("", "move", UP, gamepad_label(gamepad, NORTH), gamepad_pos, FADE_LABEL_ONLY);
    generate_action ("okcontinue", "Continue", DOWN, gamepad_label(gamepad, SOUTH), gamepad_pos, FADE_LABEL_ONLY);
  }
  else if (type == ACTION_SEL)
  {
    auto position = set<C::Absolute_position>("Action_selector", "position",
                                              value<C::Position>(CURSOR__POSITION));

    Button_orientation take_orient = LEFT_BUTTON;
    Button_orientation look_orient = RIGHT_BUTTON;
    Button_orientation move_orient = UP;
    Button_orientation inventory_orient = DOWN;

    generate_action (id, "take", take_orient, gamepad_label(gamepad, WEST), position, DEPLOY);
    generate_action (id, "look", look_orient, gamepad_label(gamepad, EAST), position, DEPLOY);
    generate_action (id, "move", move_orient, gamepad_label(gamepad, NORTH), position, DEPLOY);
    generate_action (id, "inventory", inventory_orient, gamepad_label(gamepad, SOUTH), position, DEPLOY);

    bool need_update = false;

    double overflow_down = Config::world_height - Config::label_margin
                           - 0.5 * get<C::Image>(id + "_inventory_label_back", "image")->height() * Config::interface_scale
                           - value<C::Position>(id + "_inventory_label_back", "position").y();
    if (overflow_down < 0)
    {
      need_update = true;
      if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
        position->set(Point(position->value().x(), position->value().y() + overflow_down));
      else
      {
        take_orient = LEFTER;
        look_orient = RIGHTER;
        move_orient = LEFT_UP;
        inventory_orient = RIGHT_UP;
      }
    }

    double overflow_up = value<C::Position>(id + "_move_label_back", "position").y()
                         - Config::label_margin
                         - 0.5 * get<C::Image>(id + "_move_label_back", "image")->height() * Config::interface_scale;

    if (overflow_up < 0)
    {
      need_update = true;
      if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
        position->set(Point(position->value().x(), position->value().y() - overflow_up));
      else
      {
        take_orient = LEFTER;
        look_orient = RIGHTER;
        move_orient = LEFT_DOWN;
        inventory_orient = RIGHT_DOWN;
      }
    }

    double overflow_left = value<C::Position>(id + "_take_label_back", "position").x()
                           - Config::label_margin
                           - 0.5 * get<C::Image>(id + "_take_label_back", "image")->width() * Config::interface_scale;
    if (overflow_left < 0)
    {
      need_update = true;
      if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
        position->set(Point(position->value().x() - overflow_left, position->value().y()));
      else
      {
        take_orient = UP_RIGHT;
        look_orient = DOWN_RIGHT;
        move_orient = UPPER;
        inventory_orient = DOWNER;
      }
    }

    double overflow_right = Config::world_width - Config::label_margin
                            - 0.5 * get<C::Image>(id + "_look_label_back", "image")->width() * Config::interface_scale
                            - value<C::Position>(id + "_look_label_back", "position").x();
    if (overflow_right < 0)
    {
      need_update = true;
      if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
        position->set(Point(position->value().x() + overflow_right, position->value().y()));
      else
      {
        take_orient = UP_LEFT;
        look_orient = DOWN_LEFT;
        move_orient = UPPER;
        inventory_orient = DOWNER;
      }
    }

    // If overflow on two sides, fallback to regular selector but moved away
    if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) != GAMEPAD)
      if ((overflow_up < 0 || overflow_down < 0) && (overflow_left < 0 || overflow_right < 0 ))
      {
        double dx = (overflow_left < 0 ? -overflow_left : overflow_right);
        double dy = (overflow_up < 0 ? -overflow_up : overflow_down);
        position->set(Point(position->value().x() + dx, position->value().y() + dy));
        take_orient = LEFT_BUTTON;
        look_orient = RIGHT_BUTTON;
        move_orient = UP;
        inventory_orient = DOWN;
      }

    if (need_update)
    {
      generate_action (id, "take", take_orient, gamepad_label(gamepad, WEST), position, DEPLOY);
      generate_action (id, "look", look_orient, gamepad_label(gamepad, EAST), position, DEPLOY);
      generate_action (id, "move", move_orient, gamepad_label(gamepad, NORTH), position, DEPLOY);
      generate_action (id, "inventory", inventory_orient, gamepad_label(gamepad, SOUTH), position, DEPLOY);
    }
  }
  else if (type == INV_ACTION_SEL)
  {
    Point object_pos = value<C::Position>(id , "position");
    auto position = set<C::Absolute_position>
                    ("Action_selector", "position",
                     Point (object_pos.x(),
                            value<C::Position>("Inventory", "origin").y() - 0.75 * Config::label_height * Config::interface_scale));

    generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
    double diff = value<C::Position>(id + "_combine_label_back", "position").x()
                  - 0.25 * get<C::Image>(id + "_combine_label_back", "image")->width() * Config::interface_scale
                  - (value<C::Position>("Chamfer", "position").x() + Config::label_height);
    if (diff < 0)
    {
      position->set(Point (position->value().x() - diff, position->value().y()));
      generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
    }

    generate_action (id, "look", RIGHT_BUTTON, "", position, DEPLOY);
    diff = Config::world_width
           - (value<C::Position>(id + "_look_label_back", "position").x()
              + 0.25 * get<C::Image>(id + "_look_label_back", "image")->width() * Config::interface_scale
              + Config::label_margin * Config::interface_scale);
    if (diff < 0)
    {
      position->set(Point (position->value().x() + diff, position->value().y()));
      generate_action (id, "look", RIGHT_BUTTON, "", position, DEPLOY);
      generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
    }

    generate_action (id, "use", UP, "", position, DEPLOY);
  }
  else if (type == GP_INV_ACTION_SEL)
  {
    generate_action ((get<C::Inventory>("Game", "inventory")->size() > 1 ? id : ""), "combine",
                     LEFT_BUTTON, gamepad_label(gamepad, WEST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action (id, "look", RIGHT_BUTTON, gamepad_label(gamepad, EAST), gamepad_pos, FADE_LABEL_ONLY);
    generate_action (id, "use", UP, gamepad_label(gamepad, NORTH), gamepad_pos, FADE_LABEL_ONLY);
    generate_action (id, "Cancel", DOWN, gamepad_label(gamepad, SOUTH), gamepad_pos, FADE_LABEL_ONLY);
  }

  SOSAGE_TIMER_STOP(Interface__set_action_selector);
}

void Interface::reset_action_selector()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::reset_action_selector()");

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
  SOSAGE_UPDATE_DBG_LOCATION("Interface::generate_action()");

  auto label = request<C::String>(id + "_" + action , "label");
  if (isupper(action[0]))
    label = get<C::String>(action , "text");
  if (!label)
    label = get<C::String>("Default_" + action , "label");

  Label_type ltype = PLAIN;
  Vector label_position;
  Vector button_position;
  Vector start_position;
  std::size_t selector_idx = 0;
  compute_action_positions (orientation, selector_idx, label_position,
                            button_position, start_position, ltype);

  m_action_selector[selector_idx] = id + "_" + action;

  if (style != DEPLOY)
    start_position = button_position;

  if (id != "")
  {
    std::string label_id = id + "_" + action + "_label";
    create_label (label_id, locale(label->value()), ltype, BOX);

    update_label (label_id, ltype,
                  wriggly_position(label_id, "global_position",
                                   position, label_position, orientation));

    // UPPER and DOWNER configs might need to be moved to be on screen
    if (orientation == UPPER || orientation == DOWNER)
    {
      int diff = 0;
      auto pos = get<C::Position>(label_id + "_back", "position");
      auto img =  get<C::Image>(label_id + "_back", "image");
      if (pos->value().x() - 0.5 * img->width() < Config::label_margin)
        diff = Config::label_margin - pos->value().x() + 0.5 * img->width();
      else if (pos->value().x() + 0.5 * img->width() > Config::world_width - Config::label_margin)
        diff = (Config::world_width - Config::label_margin) - pos->value().x() - 0.5 * img->width();

      if (diff != 0)
      {
        get<C::Functional_position>(label_id, "global_position")->set
            (wriggly_position(label_id, "global_position",
                              position,
                              pos->value() - position->value() + Vector(diff, 0),
                              orientation, false)->function());
      }
    }
    // LEFT_UP, RIGHT_UP, LEFT_DOWN, RIGHT_DOWN always need to be moved to not overlap
    else if (orientation == LEFT_UP)
    {
      auto pos = get<C::Position>(label_id + "_back", "position");
      double diff = Config::label_margin - get<C::Image>(label_id + "_back", "image")->width() / 2;
      get<C::Functional_position>(label_id, "global_position")->set
          (wriggly_position(label_id, "global_position",
                            position,
                            pos->value() - position->value() + Vector(diff, 0),
                            orientation, false)->function());
    }
    else if (orientation == RIGHT_UP)
    {
      auto pos = get<C::Position>(label_id + "_back", "position");
      double diff = -Config::label_margin + get<C::Image>(label_id + "_back", "image")->width() / 2;
      get<C::Functional_position>(label_id, "global_position")->set
          (wriggly_position(label_id, "global_position",
                            position,
                            pos->value() - position->value() + Vector(diff, 0),
                            orientation, false)->function());
    }
    else if (orientation == LEFT_DOWN)
    {
      auto pos = get<C::Position>(label_id + "_back", "position");
      double diff = Config::label_margin - get<C::Image>(label_id + "_back", "image")->width() / 2;
      get<C::Functional_position>(label_id, "global_position")->set
          (wriggly_position(label_id, "global_position",
                            position,
                            pos->value() - position->value() + Vector(diff, 0),
                            orientation, false)->function());
    }
    else if (orientation == RIGHT_DOWN)
    {
      auto pos = get<C::Position>(label_id + "_back", "position");
      double diff = -Config::label_margin + get<C::Image>(label_id + "_back", "image")->width() / 2;
      get<C::Functional_position>(label_id, "global_position")->set
          (wriggly_position(label_id, "global_position",
                            position,
                            pos->value() - position->value() + Vector(diff, 0),
                            orientation, false)->function());
    }

    animate_label (label_id, style);
  }

  std::string button_id = "";
  if (id == "")
  {
    m_action_selector[selector_idx] = "Default_" + action;
    button_id = "Default_" + action + "_button";
    create_label (button_id, button, LABEL_BUTTON, BOX);
    update_label (button_id, LABEL_BUTTON,
                  wriggly_position(button_id, "global_position",
                                   position, button_position, orientation));
    get<C::Position>(button_id, "global_position")->set(position->value() + start_position);

//                  set<C::Relative_position>(button_id , "global_position", position, start_position));
    if (auto img = request<C::Image>("Default_" + action + "_button", "image"))
      img->on() = false;
    get<C::Image>("Default_" + action + "_button_back", "image")->set_alpha(128);
  }
  else
  {
    button_id = id + "_" + action + "_button";
    create_label (button_id, button, LABEL_BUTTON, BOX);
    update_label (button_id, LABEL_BUTTON,
                  wriggly_position(button_id, "global_position",
                                   position, button_position, orientation));
    get<C::Position>(button_id, "global_position")->set(position->value() + start_position);
//                  set<C::Relative_position>(button_id , "global_position", position, start_position));
  }

  animate_label (button_id, style, true, position->value() + button_position);
}

void Interface::compute_action_positions (const Button_orientation& orientation,
                                          std::size_t& selector_idx,
                                          Vector& label_position, Vector& button_position,
                                          Vector& start_position, Label_type& ltype)
{
  // Default cross orientation
  if (orientation == UP)
  { selector_idx = 0;
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
    ltype = OPEN_LEFT;
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
    ltype = OPEN_RIGHT;
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
    ltype = OPEN_LEFT;
  }
  else if (orientation == DOWN_LEFT)
  {
    selector_idx = 1;
    label_position = Vector(-50, 28.25);
    button_position = Vector(-50, 28.25);
    start_position = Vector(14, 0);
    ltype = OPEN_RIGHT;
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
    ltype = OPEN_LEFT;
  }
  else if (orientation == UP_LEFT)
  {
    selector_idx = 3;
    label_position = Vector(-50, -28.25);
    button_position = Vector(-50, -28.25);
    start_position = Vector(-14, 0);
    ltype = OPEN_RIGHT;
  }
  else if (orientation == LEFTER)
  {
    selector_idx = 3;
    label_position = Vector(-60, 0);
    button_position = Vector(-60, 0);
    start_position = Vector(-14, 0);
    ltype = OPEN_RIGHT;
  }
  else if (orientation == RIGHTER)
  {
    selector_idx = 1;
    label_position = Vector(60, 0);
    button_position = Vector(60, 0);
    start_position = Vector(14, 0);
    ltype = OPEN_LEFT;
  }
  else if (orientation == LEFT_UP)
  {
    selector_idx = 0;
    label_position = Vector(-28.25, -90);
    button_position = Vector(-28.25, -50);
    start_position = Vector(0, -14);
  }
  else if (orientation == RIGHT_UP)
  {
    selector_idx = 2;
    label_position = Vector(28.25, -90);
    button_position = Vector(28.25, -50);
    start_position = Vector(0, 14);
  }
  else if (orientation == LEFT_DOWN)
  {
    selector_idx = 0;
    label_position = Vector(-28.25, 90);
    button_position = Vector(-28.25, 50);
    start_position = Vector(0, -14);
  }
  else if (orientation == RIGHT_DOWN)
  {
    selector_idx = 2;
    label_position = Vector(28.25, 90);
    button_position = Vector(28.25, 50);
    start_position = Vector(0, 14);
  }
  label_position = Config::interface_scale * label_position;
  button_position = Config::interface_scale * button_position;
  start_position = Config::interface_scale * start_position;
}

bool Interface::labels_intersect (const std::string& a, const std::string& b)
{
  auto aimg = request<C::Image>(a + "_back", "image");
  auto bimg = request<C::Image>(b + "_back", "image");

  if (!aimg || !bimg)
    return false;

  const auto& apos = value<C::Position>(a + "_back", "position");
  const auto& bpos = value<C::Position>(b + "_back", "position");

  Box boxa;
  boxa.xmin = apos.x() - aimg->width() * 0.25 * Config::interface_scale;
  boxa.xmax = apos.x() + aimg->width() * 0.25 * Config::interface_scale;
  boxa.ymin = apos.y() - aimg->height() * 0.25 * Config::interface_scale;
  boxa.ymax = apos.y() + aimg->height() * 0.25 * Config::interface_scale;
  Box boxb;
  boxb.xmin = bpos.x() - bimg->width() * 0.25 * Config::interface_scale;
  boxb.xmax = bpos.x() + bimg->width() * 0.25 * Config::interface_scale;
  boxb.ymin = bpos.y() - bimg->height() * 0.25 * Config::interface_scale;
  boxb.ymax = bpos.y() + bimg->height() * 0.25 * Config::interface_scale;

  return intersect (boxa, boxb);
}

C::Functional_position_handle Interface::wriggly_position (const std::string& id,
                                                           const std::string& cmp,
                                                           C::Position_handle origin,
                                                           const Vector& diff,
                                                           const Button_orientation& orientation,
                                                           bool insert,
                                                           bool object_label)
{
  double range = 5 * Config::interface_scale;
  constexpr double period = 0.75;
  constexpr double cos30 = 0.866025404;
  constexpr double sin30 = 0.5;

  auto time = get<C::Double>(CLOCK__TIME);
  double tbegin = std::asin(-1) - time->value() / period - Config::inventory_speed;

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  // Keep action selector of gamepad always in sync
  if (mode == GAMEPAD && !status()->is(ACTION_CHOICE))
    tbegin = 0;

  if (object_label)
  {
    // Get something close to random *but* unique per ID, to
    // keep in sync when a same label changes size for example
    tbegin = std::hash<std::string>{}(id) / double(std::numeric_limits<std::size_t>::max());
    tbegin *= 2*M_PI;
  }

  auto out = C::make_handle<C::Functional_position>
      (id, cmp,
       [range, origin, diff, time, tbegin, orientation, object_label](const std::string&) -> Point
  {
    if (object_label)
    {
      double sin_val = 0.5 * std::sin(tbegin + time->value() / period);
      if (orientation == UP)
        return origin->value() + diff + Vector (0, range * sin_val);
      else
        return origin->value() + diff + Vector (range * sin_val, 0);
    }
    else
    {
      double sin_val = 0.5 + 0.5 * std::sin(tbegin + time->value() / period);
      if (orientation == UP || orientation == UPPER)
        return origin->value() + diff + Vector (0, -range * sin_val);
      else if (orientation == DOWN || orientation == DOWNER)
        return origin->value() + diff + Vector (0, range * sin_val);
      else if (orientation == RIGHT_BUTTON || orientation == RIGHTER)
        return origin->value() + diff + Vector (range * sin_val, 0);
      else if (orientation == LEFT_BUTTON || orientation == LEFTER)
        return origin->value() + diff + Vector (-range * sin_val, 0);
      else if (orientation == DOWN_RIGHT)
        return origin->value() + diff + Vector (range * cos30 * sin_val, range * sin30 * sin_val);
      else if (orientation == DOWN_LEFT)
        return origin->value() + diff + Vector (-range * cos30 * sin_val, range * sin30 * sin_val);
      else if (orientation == UP_RIGHT)
        return origin->value() + diff + Vector (range * cos30 * sin_val, -range * sin30 * sin_val);
      else if (orientation == UP_LEFT)
        return origin->value() + diff + Vector (-range * cos30 * sin_val, -range * sin30 * sin_val);
      else if (orientation == RIGHT_UP)
        return origin->value() + diff + Vector (range * sin30 * sin_val, -range * cos30 * sin_val);
      else if (orientation == LEFT_UP)
        return origin->value() + diff + Vector (-range * sin30 * sin_val, -range * cos30 * sin_val);
      else if (orientation == RIGHT_DOWN)
        return origin->value() + diff + Vector (range * sin30 * sin_val, range * cos30 * sin_val);
      else if (orientation == LEFT_DOWN)
        return origin->value() + diff + Vector (-range * sin30 * sin_val, range * cos30 * sin_val);
    }
    return origin->value() + diff;
  }, id, true);

  if (insert)
    set(out);
  return out;
}


} // namespace Sosage::System
