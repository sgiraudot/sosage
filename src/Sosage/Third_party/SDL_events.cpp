#include <Sosage/Third_party/SDL_events.h>

#ifdef SOSAGE_LINKED_WITH_SDL

namespace Sosage::Third_party
{

SDL_events::SDL_events ()
{

}

SDL_events::~SDL_events ()
{

}

bool
SDL_events::next_event (SDL_events::Event& ev)
{
  return (SDL_PollEvent (&ev) == 1);
}

bool
SDL_events::is_exit (const Event& ev)
{
  return (ev.type == SDL_QUIT ||
          (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_ESCAPE));
}

bool
SDL_events::is_left_click (const Event& ev)
{
  return (ev.type == SDL_MOUSEBUTTONUP &&
          ev.button.type == SDL_MOUSEBUTTONUP &&
          ev.button.button == SDL_BUTTON_LEFT &&
          ev.button.state == SDL_RELEASED);
}

std::pair<int, int>
SDL_events::click_target (const Event& ev)
{
  int x = ev.button.x;
  int y = ev.button.y;
  return std::make_pair (x, y);
}

} // namespace Sosage::Third_party

#endif
