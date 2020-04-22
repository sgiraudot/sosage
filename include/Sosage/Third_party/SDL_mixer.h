#ifndef SOSAGE_THIRD_PARTY_SDL_MIXER_H
#define SOSAGE_THIRD_PARTY_SDL_MIXER_H

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

#include <SDL_mixer.h>

#include <string>

namespace Sosage::Third_party
{

class SDL_mixer
{
public:
  
  typedef Mix_Music* Music;
  typedef Mix_Chunk* Sound;
  
private:

public:

  SDL_mixer ();
  ~SDL_mixer ();

  static Music load_music (const std::string& file_name);
  static Sound load_sound (const std::string& file_name);

  static void delete_music (const Music& music);
  static void delete_sound (const Sound& sound);

  void start_music (const Music& music);
  void pause_music (const Music& music);
  void resume_music (const Music& music);
  void play_sound (const Sound& sound);
};

} // namespace Sosage::Third_party

#endif

#endif // SOSAGE_THIRD_PARTY_SDL_H
