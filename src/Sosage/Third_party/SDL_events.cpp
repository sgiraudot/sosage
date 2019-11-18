#include <Sosage/Third_party/SDL_events.h>

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
  return (ev.type == SDL_QUIT ||
          (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_ESCAPE));
}

bool SDL_events::is_pause (const Event& ev)
{
  return (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_SPACE);
}

bool SDL_events::is_debug (const Event& ev)
{
  return (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_d);
}

bool SDL_events::is_left_click (const Event& ev)
{
  return (ev.type == SDL_MOUSEBUTTONUP &&
          ev.button.type == SDL_MOUSEBUTTONUP &&
          ev.button.button == SDL_BUTTON_LEFT &&
          ev.button.state == SDL_RELEASED);
}

bool SDL_events::is_mouse_motion (const Event& ev)
{
  return (ev.type == SDL_MOUSEMOTION);
}

std::pair<int, int> SDL_events::mouse_position (const Event& ev)
{
  if (ev.type == SDL_MOUSEMOTION)
    return std::make_pair (ev.motion.x, ev.motion.y);
  // else
  return std::make_pair (ev.button.x, ev.button.y);
}

} // namespace Sosage::Third_party

