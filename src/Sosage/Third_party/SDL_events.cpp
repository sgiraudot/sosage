#include <Sosage/Third_party/SDL_events.h>
#include <Sosage/platform.h>
#include <Sosage/Config.h>

#include <iostream>

namespace Sosage::Third_party
{

SDL_events::SDL_events ()
{

}

SDL_events::~SDL_events ()
{

}

bool SDL_events::next_event (SDL_events::Event& ev)
{
  return (SDL_PollEvent (&ev) == 1);
}

bool SDL_events::is_exit (const Event& ev)
{
#ifdef SOSAGE_ANDROID
  return (ev.type == SDL_KEYUP && (ev.key.keysym.sym == SDLK_AC_BACK));
#else
  // Quit on: interface X-cross / Escape key / Q key
  return (ev.type == SDL_QUIT ||
          (ev.type == SDL_KEYUP && (ev.key.keysym.sym == SDLK_ESCAPE
                                    || ev.key.keysym.sym == SDLK_q)));
#endif
}

bool SDL_events::is_pause (const Event& ev)
{
  return (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_SPACE);
}

bool SDL_events::is_debug (const Event& ev)
{
  return (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_d);
}

bool SDL_events::is_console (const Event& ev)
{
  return (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_c);
}

bool SDL_events::is_left_click (const Event& ev)
{
#ifdef SOSAGE_ANDROID
  return ev.type == SDL_FINGERUP;
#else
  return (ev.type == SDL_MOUSEBUTTONUP &&
          ev.button.type == SDL_MOUSEBUTTONUP &&
          ev.button.button == SDL_BUTTON_LEFT &&
          ev.button.state == SDL_RELEASED);
#endif     
}

bool SDL_events::is_window_resized (const Event& ev)
{
  return ((ev.type == SDL_WINDOWEVENT &&
           ev.window.event == SDL_WINDOWEVENT_RESIZED));
}

bool SDL_events::is_mouse_motion (const Event& ev)
{
#ifdef SOSAGE_ANDROID
  return (ev.type == SDL_FINGERMOTION);
#else
  return (ev.type == SDL_MOUSEMOTION);
#endif
}

std::pair<int, int> SDL_events::mouse_position (const Event& ev)
{
#ifdef SOSAGE_ANDROID
  return std::make_pair (ev.tfinger.x * (Sosage::world_width +  config().interface_width),
                         ev.tfinger.y * (Sosage::world_height + config().interface_height));
#else
  if (ev.type == SDL_MOUSEMOTION)
    return std::make_pair (ev.motion.x, ev.motion.y);
  else if (ev.type == SDL_MOUSEBUTTONUP)
    return std::make_pair (ev.button.x, ev.button.y);
#endif
}

std::pair<int, int> SDL_events::window_size (const Event& ev)
{
  return std::make_pair (ev.window.data1, ev.window.data2);
}

} // namespace Sosage::Third_party

