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

#ifdef SOSAGE_DEV

#include <Sosage/Config/config.h>
#include <Sosage/Config/options.h>

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Test_input.h>
#include <Sosage/Utils/conversions.h>

#define BIND(x) std::bind(&Test_input::x, this)

namespace Sosage::System
{

namespace C = Component;

Test_input::Test_input (Content& content)
  : Base (content)
  , m_random_mode(false)
  , m_mode (BIND(run_mouse))
  , m_randgen(std::random_device()())
{
  set_fac<C::Simple<Vector>>(STICK__DIRECTION, "Stick", "direction", Vector(0, 0));
  get<C::Boolean>("Game", "debug")->set(true);
}

void Test_input::set_random_mode()
{
  m_random_mode = true;
  m_mode = new_mode();
}

void Test_input::run()
{
  SOSAGE_UPDATE_DBG_LOCATION("Test_input::run()");

  // Allow exiting
  while (Event ev = m_core.next_event ())
  {
    if (ev == Event(WINDOW, EXIT))
      emit ("Game", "exit");
  }

  // For scripted input, uncomment and add whatever is needed
//  static int nb = 0;
//  if (nb == 1)
//  {
//    get<C::Position> (CURSOR__POSITION)->set(cursor_target("cable"));
//    emit ("Cursor", "clicked");
//    set<C::Boolean>("Click", "left", true);
//  }
//  ++ nb;
//  return;

  if (m_random_mode && status()->is(IDLE, CUTSCENE) && ready("Test_change_mode", 10))
  {
    debug << "[TEST INPUT] Change mode" << std::endl;
    m_mode = new_mode();
  }

  m_mode();
}

void Test_input::run_mouse()
{
  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  if (mode->value() != MOUSE)
  {
    mode->set (MOUSE);
    remove("Gamepad", "id", true);
    emit("Input_mode", "changed");
  }

  if (!ready("Test_mouse_click", 0.05))
    return;

  get<C::Double>(CLOCK__LATEST_ACTIVE)->set (value<C::Double>(CLOCK__TIME));
  debug << "[TEST MOUSE] " << status()->str() << std::endl;

  if (auto action = request<C::Action>("Character", "action"))
  {
    if (action->on())
    {
      // 50% of times, let the action finish before clicking again
      auto let_finish = get_or_set<C::Boolean>("Action", "finish", random_chance(0.5));
      if (let_finish->value() == true && !status()->is(IN_MENU))
      {
        debug << "[TEST MOUSE] Waiting for action to finish" << std::endl;
        return;
      }
    }
    else
      remove ("Action", "finish", true);
  }
  else
    remove ("Action", "finish", true);

  // Generate random click in 10% of cases
  if (!status()->is(LOCKED, CUTSCENE) && (random_chance(0.1)))
  {
    Point target (random_int(0, Config::world_width), random_int(0, Config::world_height));
    debug << "[TEST MOUSE] Random click at " << target << std::endl;
    get<C::Position> (CURSOR__POSITION)->set(target);
    emit ("Cursor", "clicked");
    set<C::Boolean>("Click", "left", true);
    return;
  }

  // Escape in 1% of cases
  if (random_chance(0.01))
  {
    debug << "[TEST MOUSE] Escape" << std::endl;
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

      if (!startswith(state->value(), "inventory"))
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

      if (!startswith(state->value(), "inventory"))
        continue;
      ids.emplace_back (id);
    }
  }
  else if (status()->is(IN_WINDOW))
  {
    ids.emplace_back("background");
  }
  else if (status()->is(IN_CODE))
  {
    if (get<C::Action>("Logic", "action")->scheduled().empty())
    {
      debug << "[TEST MOUSE] Cheat code" << std::endl;
      emit("code", "cheat"); // We could maybe test keys also
    }
    else
    {
      debug << "[TEST MOUSE] Waiting end of code" << std::endl;
    }
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
    debug << "[TEST MOUSE] Escape" << std::endl;
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
    std::shuffle (ids.begin(), ids.end(), m_randgen);
    for (const std::string& id : ids)
    {
      Point target = cursor_target (id);
      if (target.is_invalid())
        continue;

      debug << "[TEST MOUSE] Click on " << id << " at " << target << std::endl;
      get<C::Position> (CURSOR__POSITION)->set(target);
      emit ("Cursor", "clicked");
      set<C::Boolean>("Click", "left", true);
      break;
    }

    ids.clear();
  }
}


void Test_input::run_mouse_chaos()
{
  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  if (mode->value() != MOUSE)
  {
    mode->set (MOUSE);
    remove("Gamepad", "id", true);
    emit("Input_mode", "changed");
  }

  if (!ready("Test_mouse_click", 0.05))
    return;

  get<C::Double>(CLOCK__LATEST_ACTIVE)->set (value<C::Double>(CLOCK__TIME));
  debug << "[TEST MOUSE CHAOS] " << status()->str() << std::endl;


  if (status()->is(LOCKED, CUTSCENE))
  {
    debug << "[TEST MOUSE CHAOS] Escape" << std::endl;
    emit ("Game", "escape");
  }
  else
  {
    Point target (random_int(0, Config::world_width), random_int(0, Config::world_height));
    debug << "[TEST MOUSE CHAOS] Random click at " << target << std::endl;
    get<C::Position> (CURSOR__POSITION)->set(target);
    emit ("Cursor", "clicked");
    set<C::Boolean>("Click", "left", true);
  }
}

void Test_input::run_touchscreen()
{
  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  if (mode->value() != TOUCHSCREEN)
  {
    mode->set (TOUCHSCREEN);
    remove("Gamepad", "id", true);
    emit("Input_mode", "changed");
  }

  if (!ready("Test_mouse_click", 0.05))
    return;

  get<C::Double>(CLOCK__LATEST_ACTIVE)->set (value<C::Double>(CLOCK__TIME));
  debug << "[TEST TOUCH] " << status()->str() << std::endl;

  if (auto action = request<C::Action>("Character", "action"))
  {
    if (action->on())
    {
      // 50% of times, let the action finish before clicking again
      auto let_finish = get_or_set<C::Boolean>("Action", "finish", random_chance(0.5));
      if (let_finish->value() == true && !status()->is(IN_MENU))
      {
        debug << "[TEST TOUCH] Waiting for action to finish" << std::endl;
        return;
      }
    }
    else
      remove ("Action", "finish", true);
  }
  else
    remove ("Action", "finish", true);

  if (auto path = request<C::Path>(value<C::String>("Player", "name", ""), "path"))
  {
    // 20% of times, let the path finish before clicking again
    auto let_finish = get_or_set<C::Boolean>("Path", "finish", random_chance(0.2));
    if (let_finish->value() == true && !status()->is(IN_MENU))
    {
      debug << "[TEST TOUCH] Waiting for pathto finish" << std::endl;
      return;
    }
    else
      remove ("Path", "finish", true);
  }
  else
    remove ("Path", "finish", true);

  // Generate random click in 10% of cases
  if (!status()->is(LOCKED, CUTSCENE) && (random_chance(0.1)))
  {
    Point target (random_int(0, Config::world_width), random_int(0, Config::world_height));
    debug << "[TEST TOUCH] Random click at " << target << std::endl;
    get<C::Position> (CURSOR__POSITION)->set(target);
    emit ("Cursor", "clicked");
    set<C::Boolean>("Click", "left", true);
    return;
  }

  // Escape in 1% of cases
  if (random_chance(0.01))
  {
    debug << "[TEST TOUCH] Escape" << std::endl;
    emit ("Game", "escape");
    return;
  }

  static std::vector<std::string> ids;
  if (status()->is(IDLE))
  {
    auto active_objects = request<C::Vector<std::string>>("Interface", "active_objects");
    if (active_objects && random_chance(0.5))
    {
      for (C::Handle c : components("image"))
        if (auto img = C::cast<C::Image>(c))
        {
          if (!img->on())
            continue;
          std::string id = img->entity();
          std::size_t pos = id.find("_label");
          if (pos != std::string::npos)
            id.resize(pos);
          if (!request<C::String>(id , "name"))
            continue;
          if (!contains(active_objects->value(), id))
            continue;
          ids.emplace_back (c->entity());
        }
    }
    else
    {
      Point target (random_int(0, Config::world_width), random_int(0, Config::world_height));
      debug << "[TEST TOUCH] Random click at " << target << std::endl;
      get<C::Position> (CURSOR__POSITION)->set(target);
      emit ("Cursor", "clicked");
      set<C::Boolean>("Click", "left", true);
      return;
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

      if (!startswith(state->value(), "inventory"))
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

      if (!startswith(state->value(), "inventory"))
        continue;
      ids.emplace_back (id);
    }
  }
  else if (status()->is(IN_WINDOW))
  {
    ids.emplace_back("background");
  }
  else if (status()->is(IN_CODE))
  {
    if (get<C::Action>("Logic", "action")->scheduled().empty())
    {
      debug << "[TEST TOUCH] Cheat code" << std::endl;
      emit("code", "cheat"); // We could maybe test keys also
    }
    else
    {
      debug << "[TEST TOUCH] Waiting end of code" << std::endl;
    }
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
    debug << "[TEST TOUCH] Escape" << std::endl;
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
    std::shuffle (ids.begin(), ids.end(), m_randgen);
    for (const std::string& id : ids)
    {
      Point target = cursor_target (id);
      if (target.is_invalid())
        continue;

      debug << "[TEST TOUCH] Click on " << id << " at " << target << std::endl;
      get<C::Position> (CURSOR__POSITION)->set(target);
      emit ("Cursor", "clicked");
      set<C::Boolean>("Click", "left", true);
      break;
    }

    ids.clear();
  }
}

void Test_input::run_gamepad()
{
  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  if (mode->value() != GAMEPAD)
  {
    mode->set (GAMEPAD);
    set<C::Simple<Gamepad_info>>("0:0", "gamepad", Gamepad_info());
    set<C::String>("Gamepad", "id", "0:0");
    emit("Input_mode", "changed");
  }

  if (!ready("Test_mouse_click", 0.05))
    return;
  debug << "[TEST GAMEPAD] " << status()->str() << std::endl;

  get<C::Double>(CLOCK__LATEST_ACTIVE)->set(value<C::Double>(CLOCK__TIME));

  // Escape in 1% of cases
  if (random_chance(0.01))
  {
    debug << "[TEST GAMEPAD] Escape" << std::endl;
    emit ("Game", "escape");
    return;
  }

  if (ready("Test_stick_dir", 3.))
  {
    Vector vec (random_double(-1, 1.), random_double(-1., 1));
    vec.normalize();
    get<C::Simple<Vector>>(STICK__DIRECTION)->set(vec);
    debug << "[TEST GAMEPAD] Move stick to " << vec << std::endl;
  }

  bool right = random_chance(0.5);
  std::string key = random_choice({"move", "take", "look", "inventory"});

  if (status()->is(IDLE))
  {
    if (random_chance(0.25))
    {
      auto active_objects = request<C::Vector<std::string>>("Interface", "active_objects");
      if (active_objects)
      {
        if (random_chance(0.75))
        {
          debug << "[TEST GAMEPAD] Switch " << (right ? "right" : "left") << std::endl;
          set<C::Boolean>("Switch", "right", random_chance(0.5));
        }
        else
        {
          debug << "[TEST GAMEPAD] Key " << key << std::endl;
          emit("Action", key);
        }
      }
      else if (random_chance(0.1))
      {
        debug << "[TEST GAMEPAD] Key inventory" << std::endl;
        emit("Action", "inventory");
      }
    }
    emit("Stick", "moved");
  }
  else if (status()->is(IN_INVENTORY, IN_MENU, DIALOG_CHOICE))
  {
    if (random_chance(0.75))
    {
      debug << "[TEST GAMEPAD] Switch " << (right ? "right" : "left") << std::endl;
      set<C::Boolean>("Switch", "right", random_chance(0.5));
    }
    else
    {
      debug << "[TEST GAMEPAD] Key " << key << std::endl;
      emit("Action", key);
    }
  }
  else if (status()->is(ACTION_CHOICE, INVENTORY_ACTION_CHOICE, OBJECT_CHOICE))
  {
    if (random_chance(0.25))
    {
      debug << "[TEST GAMEPAD] Switch " << (right ? "right" : "left") << std::endl;
      set<C::Boolean>("Switch", "right", random_chance(0.5));
    }
    else
    {
      debug << "[TEST GAMEPAD] Key " << key << std::endl;
      emit("Action", key);
    }
  }
  else if (status()->is(IN_WINDOW))
  {
    debug << "[TEST GAMEPAD] Key " << key << std::endl;
    emit("Action", key);
  }
  else if (status()->is(IN_CODE))
  {
    if (get<C::Action>("Logic", "action")->scheduled().empty())
    {
      debug << "[TEST GAMEPAD] Cheat code" << std::endl;
      emit("code", "cheat"); // We could maybe test keys also
    }
    else
    {
      debug << "[TEST GAMEPAD] Waiting end of code" << std::endl;
    }
  }
  else if (status()->is(LOCKED, CUTSCENE))
  {
    // Skip dialogs if possible
    debug << "[TEST GAMEPAD] Escape" << std::endl;
    emit("Game", "escape");
  }
}

void Test_input::run_gamepad_chaos()
{
  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  if (mode->value() != GAMEPAD)
  {
    mode->set (GAMEPAD);
    remove("Gamepad", "id", true);
    emit("Input_mode", "changed");
  }

  if (!ready("Test_mouse_click", 0.05))
    return;
  debug << "[TEST GAMEPAD] " << status()->str() << std::endl;

  get<C::Double>(CLOCK__LATEST_ACTIVE)->set(value<C::Double>(CLOCK__TIME));

  if (status()->is(LOCKED, CUTSCENE))
  {
    debug << "[TEST GAMEPAD CHAOS] Escape" << std::endl;
    emit ("Game", "escape");
  }
  else
  {
    Vector vec (random_double(-1, 1.), random_double(-1., 1));
    vec.normalize();
    if (random_chance(0.25))
      vec = Vector (0., 0.);
    debug << "[TEST GAMEPAD CHAOS] Move stick to " << vec << std::endl;
    get<C::Simple<Vector>>(STICK__DIRECTION)->set(vec);

    random_do ({ [&] { debug << "[TEST GAMEPAD CHAOS] Switch right" << std::endl;
                       set<C::Boolean>("Switch", "right", true); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Switch left" << std::endl;
                        set<C::Boolean>("Switch", "right", false); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Escape" << std::endl;
                       emit ("Game", "escape"); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Key move" << std::endl;
                       emit ("Action", "move"); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Key take" << std::endl;
                       emit ("Action", "take"); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Key look" << std::endl;
                       emit ("Action", "look"); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Key inventory" << std::endl;
                       emit ("Action", "inventory"); },
                 [&] { debug << "[TEST GAMEPAD CHAOS] Stick moved" << std::endl;
                       emit ("Stick", "moved"); } });
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

  Point screen_position = p - img->scale() * Vector(img->origin());

  double xmin_target = zoom * screen_position.x();
  double ymin_target = zoom * screen_position.y();
  double xmax_target = zoom * (screen_position.x() + img->scale() * (xmax - xmin));
  double ymax_target = zoom * (screen_position.y() + img->scale() * (ymax - ymin));

  // Skip out of boundaries images
  if ((xmax_target < 0 || xmin_target > limit_width)
      && (ymax_target < 0 || ymin_target > limit_height))
    return Point::invalid();

  return Point(0.5 * (xmin_target + xmax_target),
               0.5 * (ymin_target + ymax_target));
}

std::function<void(void)> Test_input::new_mode()
{
  return random_choice ({ BIND(run_mouse),
                          BIND(run_mouse_chaos),
                          BIND(run_touchscreen),
                          BIND(run_gamepad),
                          BIND(run_gamepad_chaos) });
}

bool Test_input::ready (const std::string& key, double time)
{
  double now = value<C::Double>(CLOCK__TIME);
  auto then = get_or_set<C::Double>(key, "time", now);
  double ellapsed = now - then->value();
  if (ellapsed < time)
    return false;

  then->set(now);
  return true;
}

} // namespace Sosage::System

#endif
