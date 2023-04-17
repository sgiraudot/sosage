/*
  [src/Sosage/System/Input.cpp]
  Reads and interprets user input (keyboard, mouse, touchscreen).

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
#include <Sosage/Component/Condition.h>
#include <Sosage/Component/GUI_animation.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/config.h>
#include <Sosage/System/Input.h>

#include <map>

namespace Sosage::System
{

namespace C = Component;

Input::Input (Content& content)
  : Base (content)
  , m_core()
  , m_keys_on(NUMBER_OF_EVENT_VALUES, false)
#ifdef SOSAGE_DEV
  , m_fake_touchscreen(false)
  , m_demo_mode(false)
  , m_randgen(std::random_device()())
#endif
{
  set_fac<C::Simple<Vector>>(STICK__DIRECTION, "Stick", "direction", Vector(0, 0));
}

void Input::run()
{
  SOSAGE_TIMER_START(System_Input__run);
  SOSAGE_UPDATE_DBG_LOCATION("Input::run()");

#ifdef SOSAGE_DEV
  bool keyboard_used = false;
#endif
  bool mouse_used = false;
  bool touchscreen_used = false;
  bool gamepad_used = false;

  while (Event ev = m_core.next_event ())
  {
#ifdef SOSAGE_DEV
    if ((ev.type() == KEY_DOWN || ev.type() == KEY_UP)
        && (ev.value() == UP_ARROW || ev.value() == RIGHT_ARROW ||
            ev.value() == LEFT_ARROW || ev.value() == DOWN_ARROW ||
            ev.value() == TAB || ev.value() == I || ev.value() == J ||
            ev.value() == K ||ev.value() == L))
      keyboard_used = true;
    else
#endif
    if (ev.type() == MOUSE_DOWN || ev.type() == MOUSE_UP)
      mouse_used = true;
    else if (ev.type() == TOUCH_DOWN || ev.type() == TOUCH_MOVE || ev.type() == TOUCH_UP)
      touchscreen_used = true;
    else if (ev.type() == NEW_GAMEPAD || ev.type() == BUTTON_DOWN ||
             ev.type() == BUTTON_UP || ev.type() == STICK_MOVE)
      gamepad_used = true;

#ifdef SOSAGE_DEV
    if (ev.type() == KEY_DOWN && ev.value() == T)
    {
      if (m_fake_touchscreen)
      {
        receive("Fake_touchscreen", "enabled");
        mouse_used = true;
      }
      else
        emit("Fake_touchscreen", "enabled");
      m_fake_touchscreen = !m_fake_touchscreen;
    }
#endif

#if defined(SOSAGE_DEV) || defined(SOSAGE_DEMO)
    if (ev.type() == KEY_UP && ev.value() == A)
      m_demo_mode = true;
    else
      m_demo_mode = false;
#endif

    if (ev.type() != UNUSED)
      m_current_events.emplace_back(ev);
  }

#if defined(SOSAGE_DEV) || defined(SOSAGE_DEMO)
  if (m_demo_mode)
  {
    m_current_events.clear();
    run_demo_mode();
    SOSAGE_TIMER_STOP(System_Input__run);
    return;
  }
#endif

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  auto gamepad = get<C::Simple<Gamepad_type>>(GAMEPAD__TYPE);

  // Only allow mode change when idle or cutscene
  if (status()->is(IDLE, CUTSCENE))
  {
    Input_mode previous_mode = mode->value();
    if (touchscreen_used
#ifdef SOSAGE_DEV
        || m_fake_touchscreen
#endif
        )
    {
      mode->set (TOUCHSCREEN);
      gamepad->set (NO_LABEL);
    }
    else if (mouse_used)
    {
      mode->set (MOUSE);
      gamepad->set (NO_LABEL);
    }
#ifdef SOSAGE_DEV
    else if (keyboard_used)
    {
      mode->set (GAMEPAD);
      gamepad->set (KEYBOARD);
    }
#endif
    else if (gamepad_used)
    {
      mode->set (GAMEPAD);
      if (previous_mode != GAMEPAD)
        gamepad->set (m_core.gamepad_type());
    }

    if (previous_mode != mode->value())
      emit("Input_mode", "changed");
  }

  if (mode->value() == GAMEPAD)
  {
    if (gamepad_used
#ifdef SOSAGE_DEV
        || keyboard_used
#endif
        || value<C::Simple<Vector>>(STICK__DIRECTION) != Vector(0,0))
      get<C::Double>(CLOCK__LATEST_ACTIVE)->set(value<C::Double>(CLOCK__TIME));
  }

  bool arrow_released = false;
  for (const Event& ev : m_current_events)
  {
    if (ev == Event(KEY_UP, ESCAPE) ||
        ev == Event(KEY_UP, ANDROID_BACK) ||
        ev == Event(BUTTON_UP, START) ||
        ev == Event(BUTTON_UP, SELECT))
      emit ("Game", "escape");

    if (ev == Event(WINDOW, EXIT))
      emit ("Game", "exit");

    // Some ways to skip dialogs
    if (status()->is(LOCKED))
    {
      if (ev == Event(KEY_UP, SPACE)
          || ev == Event(BUTTON_DOWN, EAST)
          || ev == Event(BUTTON_DOWN, SOUTH)
          || ev == Event(TOUCH_DOWN, LEFT)
          || ev == Event(MOUSE_DOWN, RIGHT))
        emit ("Game", "skip_dialog");
    }

    // Some ways to skip notifications
    if (ev == Event(KEY_UP, SPACE)
        || ev == Event(BUTTON_DOWN, EAST)
        || ev == Event(BUTTON_DOWN, SOUTH)
        || ev == Event(TOUCH_DOWN, LEFT)
        || ev == Event(MOUSE_DOWN, RIGHT))
      emit ("Game", "clear_notifications");

    // Speeding up game
    if (ev == Event(MOUSE_DOWN, RIGHT) or ev == Event(KEY_DOWN, SPACE))
      emit ("Time", "begin_speedup");
    else if (ev == Event(MOUSE_UP, RIGHT) or ev == Event(KEY_UP, SPACE))
    {
      if (request<C::Double>("Speedup", "begin"))
        emit ("Time", "end_speedup");
    }

    if (ev == Event(WINDOW, FOREGROUND)
        && status()->is(PAUSED))
      status()->pop();
    if (ev == Event(WINDOW, BACKGROUND)
        && !status()->is (PAUSED))
      status()->push(PAUSED);

    if (status()->is (PAUSED))
      continue;

#ifndef SOSAGE_RELEASE
    if (ev == Event(KEY_UP, D))
      get<C::Boolean>("Game", "debug")->toggle();
#endif

#ifdef SOSAGE_DEV
    if (ev == Event(KEY_UP, N))
      emit("Game", "test");
#endif

#ifdef SOSAGE_PROFILE
    if (ev == Event(KEY_UP, P))
    {
      std::size_t nb_small = 0;
      std::size_t i = 0;
      for (const auto& comp : m_content)
      {
        debug << i << ": " << comp.size() << std::endl;
        if (comp.size() <= 3)
          ++ nb_small;
      }
      debug << m_content.size() << " UNIQUE COMPONENTS" << std::endl;
      debug << "(" << nb_small << " small ones -> "
            << 100. * nb_small / m_content.size() << "%)" << std::endl;
    }
#endif

    if (ev == Event(KEY_UP, Sosage::C))
    {
#if 0
      for (const auto& cmp : m_content)
        for (const auto& c : cmp)
        {
          debug << component_str(c.second, 0);
        }
#endif

#ifdef SOSAGE_DEV
      debug << "> " << std::endl;
      std::string text_input;
      std::getline (std::cin, text_input);
      set<C::String>("Test", "console_action", text_input);
#endif
    }

    if constexpr (!Config::emscripten) // Do not prevent web users to use F1/F2/etc
    {
      if (ev == Event(KEY_DOWN, ALT))
        key_on(ALT) = true;
      else if (ev == Event(KEY_UP, ALT))
        key_on(ALT) = false;
      else if (ev == Event(KEY_UP, ENTER) && key_on(ALT))
      {
        get<C::Boolean>("Window", "fullscreen")->toggle();
        emit ("Window", "toggle_fullscreen");
      }
    }
    if (ev == Event(WINDOW, RESIZED))
    {
      get<C::Int>("Window", "width")->set(ev.x());
      get<C::Int>("Window", "height")->set(ev.y());
      emit ("Window", "rescaled");
    }

    if (ev.type() == MOUSE_MOVE
        && (mode->value() == MOUSE
            || (mode->value() == TOUCHSCREEN
#ifdef SOSAGE_DEV
                && m_fake_touchscreen
#endif
                )))
      get<C::Position>
          (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));

    // If locked/cutscene, ignore mouse clicks
    if (status()->is (LOCKED, CUTSCENE))
      continue;

    if (mode->value() == MOUSE)
    {
      if (ev == Event(MOUSE_DOWN, LEFT))
      {
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        emit ("Cursor", "clicked");
      }
    }
    else if (mode->value() == TOUCHSCREEN)
    {
#ifdef SOSAGE_DEV
      if (m_fake_touchscreen) // Simulate touchscreen with mouse for testing
      {
        if (ev == Event(MOUSE_DOWN, LEFT))
        {
          get<C::Position>
              (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
          emit ("Cursor", "clicked");
        }
      }
      else // Real touchscreen
#endif
      {
        if (ev == Event(TOUCH_DOWN, LEFT))
        {
          get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
          emit ("Cursor", "clicked");
        }
      }
    }
    else // if (mode->value() == GAMEPAD)
    {
#ifdef SOSAGE_DEV
      if (gamepad->value() == KEYBOARD)
      {
        if (ev.type() == KEY_DOWN)
        {
          key_on(ev.value()) = true;
          if (ev.value() == TAB)
            set<C::Boolean>("Switch", "right", true);
          else if (ev.value() == I)
            emit("Action", "move");
          else if (ev.value() == J)
            emit("Action", "take");
          else if (ev.value() == L)
            emit("Action", "look");
          else if (ev.value() == K)
            emit("Action", "inventory");
        }
        else if (ev.type() == KEY_UP)
        {
          key_on(ev.value()) = false;
          if (ev.value() == UP_ARROW
              || ev.value() == DOWN_ARROW
              || ev.value() == LEFT_ARROW
              || ev.value() == RIGHT_ARROW)
            arrow_released = true;
        }
      }
      else // Real gamepad (no keyboard)
#endif
      {
        if (ev == Event (STICK_MOVE, LEFT))
        {
          if (ev.y() == Config::no_value)
            m_x = ev.x() / Config::stick_max;
          if (ev.x() == Config::no_value)
            m_y = ev.y() / Config::stick_max;
        }
        else if (ev.type() == BUTTON_DOWN)
        {
          key_on(ev.value()) = true;
          if (ev.value() == LEFT_SHOULDER)
            set<C::Boolean>("Switch", "right", false);
          else if (ev.value() == RIGHT_SHOULDER)
            set<C::Boolean>("Switch", "right", true);
          else if (ev.value() == NORTH)
            emit("Action", "move");
          else if (ev.value() == WEST)
            emit("Action", "take");
          else if (ev.value() == EAST)
            emit("Action", "look");
          else if (ev.value() == SOUTH)
            emit("Action", "inventory");
        }
        else if (ev.type() == BUTTON_UP)
        {
          key_on(ev.value()) = false;
          if (ev.value() == LEFT_SHOULDER || ev.value() == RIGHT_SHOULDER)
            if (request<C::Double>("Speedup", "begin"))
              emit ("Time", "end_speedup");
        }
      }

    }
  }

  if (mode->value() == GAMEPAD)
  {
    // Speed-up
    if (key_on(RIGHT_SHOULDER) && key_on(LEFT_SHOULDER))
      if (!request<C::Double>("Speedup", "begin"))
        emit ("Time", "begin_speedup");

    // If D-PAD is used, ignore stick
    if (arrow_released || key_on(UP_ARROW)
        || key_on(DOWN_ARROW) || key_on(LEFT_ARROW)
        || key_on(RIGHT_ARROW))
    {
      m_x = 0;
      m_y = 0;

      // Use DPAD if stick is not used only
      if (key_on(UP_ARROW)) m_y = -1.;
      if (key_on(DOWN_ARROW)) m_y = 1.;
      if (key_on(LEFT_ARROW)) m_x = -1.;
      if (key_on(RIGHT_ARROW)) m_x = 1.;
    }

    Vector vec (m_x, m_y);
    if (vec.length() > 0.5)
      vec.normalize();
    else // deadzone
      vec = Vector(0,0);

    auto stick_direction = get<C::Simple<Vector>>(STICK__DIRECTION);
    if (vec != stick_direction->value())
    {
      stick_direction->set(vec);
      debug << " Stick moved to " << vec << std::endl;
      debug << m_x << " " << m_y << std::endl;
      emit("Stick", "moved");
    }
  }

  m_current_events.clear();
  SOSAGE_TIMER_STOP(System_Input__run);
}

typename std::vector<bool>::reference Input::key_on(const Event_value& value)
{
  return m_keys_on[std::size_t(value)];
}

#if defined(SOSAGE_DEV) || defined(SOSAGE_DEMO)
void Input::run_demo_mode()
{
  if (status()->is(LOCKED, CUTSCENE))
    return;

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  auto gamepad = get<C::Simple<Gamepad_type>>(GAMEPAD__TYPE);
  if (mode->value() != MOUSE)
  {
    mode->set (MOUSE);
    gamepad->set (NO_LABEL);
    emit("Input_mode", "changed");
  }

  // Let actions finish
  if (auto action = request<C::Action>("Character", "action"))
    if (action->on())
        return;

  const std::string& player = value<C::String>("Player", "name");

  // Let path finish
  if (request<C::Path>(player, "path"))
    return;

  double current_time = value<C::Double>(CLOCK__TIME);

  // Let current demo input finish
  if (auto wakeup = request<C::Double>("Demo", "wake_up_time"))
    if (wakeup->value() > current_time)
      return;

  // Click after moving
  if (receive("Demo", "cursor_moved"))
  {
    emit ("Cursor", "clicked");
    set<C::Double>("Demo", "wake_up_time", current_time + 0.1);
    return;
  }

  Point target = Point::invalid();

  // Generate random click in 5% of cases
#if 1
  if (!status()->is(LOCKED, CUTSCENE) && (random_chance(0.05)))
      target = Point (random_int(50, Config::world_width - 50),
                      random_int(50, Config::world_height - 50));
  else
#endif
  {
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

          // Do not change settings or quit
          if (startswith(img->entity(), "Settings")
              || startswith(img->entity(), "Save_and_quit"))
            continue;

          ids.emplace_back (c->entity());
        }
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
        target = cursor_target (id);
        debug << "Trying target " << id << " -> " << target << std::endl;
        if (!target.is_invalid())
        {
          debug << " -> ok " << std::endl;
          break;
        }
      }
      ids.clear();
    }
    else
    {
      debug << "No target available" << std::endl;
    }
  }

  if (target.is_invalid())
    target = Point (random_int(50, Config::world_width - 50),
                    random_int(50, Config::world_height - 50));

  auto cursor_pos = get<C::Position>(CURSOR__POSITION);
  set<C::GUI_position_animation>("Cursor", "animation", current_time, current_time + 1,
                                 cursor_pos, target);
  set<C::Double>("Demo", "wake_up_time", current_time + 1.1);
  emit("Demo", "cursor_moved");
}

// Copy-pasted from Test_input, should be factorized
Point Input::cursor_target (const std::string& id)
{
  const Point& camera = value<C::Absolute_position>(CAMERA__POSITION);
  double limit = 20;
  double limit_width = Config::world_width - 50;
  double limit_height = Config::world_height - 50;

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
  if (xmax_target < limit || xmin_target > limit_width
      || ymax_target < limit || ymin_target > limit_height)
    return Point::invalid();

  xmin_target = std::max(xmin_target, limit);
  ymin_target = std::max(ymin_target, limit);
  xmax_target = std::min(xmax_target, limit_width);
  ymax_target = std::min(ymax_target, limit_height);

  return Point(0.5 * (xmin_target + xmax_target),
               0.5 * (ymin_target + ymax_target));
}
#endif


} // namespace Sosage::System
