#ifndef SOSAGE_THIRD_PARTY_SDL_EVENTS_H
#define SOSAGE_THIRD_PARTY_SDL_EVENTS_H

#include <utility>

#include <SDL2/SDL.h>

namespace Sosage::Third_party
{

class SDL_events
{
public:

  typedef SDL_Event Event;
  
private:
  
public:

  SDL_events();

  ~SDL_events();

  bool next_event(Event& ev);

  bool is_exit (const Event& ev);
  bool is_left_click (const Event& ev);
  bool is_mouse_motion (const Event& ev);
  std::pair<int, int> mouse_position (const Event& ev);
};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_EVENTS_H
