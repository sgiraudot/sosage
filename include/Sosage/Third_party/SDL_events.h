#ifndef SOSAGE_THIRD_PARTY_SDL_EVENTS_H
#define SOSAGE_THIRD_PARTY_SDL_EVENTS_H

#include <Sosage/platform.h>

#include <utility>

#ifdef SOSAGE_ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

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
  bool is_pause (const Event& ev);
  bool is_debug (const Event& ev);
  bool is_console (const Event& ev);
  bool is_left_click (const Event& ev);
  bool is_mouse_motion (const Event& ev);
  bool is_window_resized (const Event& ev);
  std::pair<int, int> mouse_position (const Event& ev);
  std::pair<int, int> window_size (const Event& ev);
};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_EVENTS_H
