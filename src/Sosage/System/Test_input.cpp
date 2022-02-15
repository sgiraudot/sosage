/*
  [src/Sosage/System/Test_input.cpp]
  Automatic input generation for testing purposes.

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

#include <Sosage/Config/config.h>
#include <Sosage/Config/options.h>

#ifdef SOSAGE_TEST_INPUT

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Test_input.h>
#include <Sosage/Utils/conversions.h>

namespace Sosage::System
{

namespace C = Component;

Test_input::Test_input (Content& content)
  : Base (content)
{
  set_fac<C::Simple<Vector>>(STICK__DIRECTION, "Stick", "direction", Vector(0, 0));
  get<C::Boolean>("Game", "debug")->set(true);
}

void Test_input::run()
{
  // Allow exiting
  while (Event ev = m_core.next_event ())
  {
    if (ev == Event(WINDOW, EXIT))
      emit ("Game", "exit");
  }

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  auto gamepad = get<C::Simple<Gamepad_type>>(GAMEPAD__TYPE);
  if (mode->value() != MOUSE)
  {
    mode->set (MOUSE);
    gamepad->set (NO_LABEL);
    emit("Input_mode", "changed");
  }

  double inactive_time = value<C::Double>(CLOCK__TIME) - value<C::Double>(CLOCK__LATEST_ACTIVE);
  if (inactive_time < 0.05)
    return;

  get<C::Double>(CLOCK__LATEST_ACTIVE)->set (value<C::Double>(CLOCK__TIME));
  debug << "[TEST INPUT] " << status()->str() << std::endl;

  if (auto action = request<C::Action>("Character", "action"))
  {
    if (action->on())
    {
      // 50% of times, let the action finish before clicking again
      auto let_finish = get_or_set<C::Boolean>("Action", "finish", random_chance(0.5));
      if (let_finish->value() == true && !status()->is(IN_MENU))
      {
        debug << "[TEST INPUT] Waiting for action to finish" << std::endl;
        return;
      }
    }
    else
      remove ("Action", "finish", true);
  }
  else
    remove ("Action", "finish", true);

  // Generate random click in 10% of cases
  if (!status()->is(LOCKED) && (random_chance(0.1)))
  {
    Point target (random_int(0, Config::world_width), random_int(0, Config::world_height));
    debug << "[TEST INPUT] Random click at " << target << std::endl;
    get<C::Position> (CURSOR__POSITION)->set(target);
    emit ("Cursor", "clicked");
    set<C::Boolean>("Click", "left", true);
    return;
  }

  // Escape in 1% of cases
  if (random_chance(0.01))
  {
    debug << "[TEST INPUT] Escape" << std::endl;
    emit ("Game", "escape");
    return;
  }

  static std::vector<std::string> ids;
  if (status()->is(IDLE))
  {
    for (C::Handle c : components("name"))
    {
      auto img = request<C::Image>(c->entity(), "image");
      if (!img || !img->on())
        continue;
      ids.emplace_back (c->entity());
    }

  }
  else if (status()->is(IN_INVENTORY))
  {
    for (C::Handle c : components("name"))
    {
      auto img = request<C::Image>(c->entity(), "image");
      if (!img || !img->on())
        continue;
      if (!img->on())
        continue;

      std::string id = c->entity();

      auto state = request<C::String>(id , "state");
      if (!state)
        continue;
      if (auto source = request<C::String>("Interface", "source_object"))
        if (source->value() == id)
          continue;

      if (state->value() != "inventory")
        continue;

      ids.emplace_back (id);
    }
  }
  else if (status()->is(ACTION_CHOICE, INVENTORY_ACTION_CHOICE))
  {
    for (C::Handle c : components("image"))
      if (auto img = C::cast<C::Image>(c))
      {
        if (!img->on())
          continue;
        if (!contains(img->entity(), "_label") && !contains(img->entity(), "_button"))
          continue;
        ids.emplace_back (c->entity());
      }
  }
  else if (status()->is(OBJECT_CHOICE))
  {
    for (C::Handle c : components("name"))
    {
      auto img = request<C::Image>(c->entity(), "image");
      if (!img || !img->on())
        continue;
      if (!img->on())
        continue;

      std::string id = c->entity();

      auto state = request<C::String>(id , "state");
      if (!state)
        continue;

      if (state->value() != "inventory")
        continue;
      ids.emplace_back (id);
    }
  }
  else if (status()->is(IN_WINDOW))
  {
    ids.emplace_back("Background");
  }
  else if (status()->is(IN_CODE))
  {
    emit("code", "cheat"); // We could maybe test keys also
  }
  else if (status()->is(IN_MENU))
  {
    for (C::Handle c : components("image"))
      if (auto img = C::cast<C::Image>(c))
      {
        if (!img->on())
          continue;
        if (!contains(img->entity(), "_button") && !contains(img->entity(), "_arrow"))
          continue;

        ids.emplace_back (c->entity());
      }
  }
  else if (status()->is(LOCKED, CUTSCENE))
  {
    // Skip dialogs if possible
    debug << "[TEST INPUT] Escape" << std::endl;
    emit("Game", "escape");
    return;
  }
  else if (status()->is(DIALOG_CHOICE))
  {
    for (C::Handle c : components("image"))
      if (auto img = C::cast<C::Image>(c))
      {
        if (!img->on())
          continue;
        if (!contains(img->entity(), "Dialog_choice_") && !contains(img->entity(), "background"))
          continue;
        ids.emplace_back (c->entity());
      }
  }

  if (!ids.empty())
  {
    std::random_shuffle (ids.begin(), ids.end());
    for (const std::string& id : ids)
    {
      Point target = cursor_target (id);
      if (target == Point::invalid())
        continue;

      debug << "[TEST INPUT] Click on " << id << " at " << target << std::endl;
      get<C::Position> (CURSOR__POSITION)->set(target);
      emit ("Cursor", "clicked");
      set<C::Boolean>("Click", "left", true);
      break;
    }

    ids.clear();
  }


}

Point Test_input::cursor_target (const std::string& id)
{
  const Point& camera = value<C::Absolute_position>(CAMERA__POSITION);
  double limit_width = Config::world_width;
  double limit_height = Config::world_height;

  auto img = get<C::Image>(id, "image");
  auto position = get<C::Position>(id, "position");

  Point p = position->value();
  double zoom = 1.;
  if (!position->is_interface())
  {
    p = p - camera;
    zoom = value<C::Double>(CAMERA__ZOOM);
  }

  int xmin = img->xmin();
  int ymin = img->ymin();
  int xmax = img->xmax();
  int ymax = img->ymax();

  Point screen_position = p - img->core().scaling * Vector(img->origin());

  double xmin_target = zoom * screen_position.x();
  double ymin_target = zoom * screen_position.y();
  double xmax_target = zoom * (screen_position.x() + img->core().scaling * (xmax - xmin));
  double ymax_target = zoom * (screen_position.y() + img->core().scaling * (ymax - ymin));

  // Skip out of boundaries images
  if ((xmax_target < 0 || xmin_target > limit_width)
      && (ymax_target < 0 || ymin_target > limit_height))
    return Point::invalid();

  return Point(0.5 * (xmin_target + xmax_target),
               0.5 * (ymin_target + ymax_target));
}

} // namespace Sosage::System

#endif // SOSAGE_TEST_INPUT
