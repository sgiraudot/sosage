#include <Sosage/Third_party/SDL_mixer.h>
#include <Sosage/Utils/error.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

namespace Sosage::Third_party
{

SDL_mixer::SDL_mixer()
{
  int init = Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
  check (init != -1, "Cannot initialized SDL Mixer");
  Mix_AllocateChannels (10);
}

SDL_mixer::~SDL_mixer()
{
  Mix_CloseAudio ();
}

SDL_mixer::Music SDL_mixer::load_music (const std::string& file_name)
{
  Mix_Music* music = Mix_LoadMUS (file_name.c_str());
  Mix_VolumeMusic(64);
  check (music != nullptr, "Cannot load music " + file_name);
  return music;
}

SDL_mixer::Sound SDL_mixer::load_sound (const std::string& file_name)
{
  Mix_Chunk* sound = Mix_LoadWAV (file_name.c_str());
  check (sound != nullptr, "Cannot load sound " + file_name);
  return sound;
}

void SDL_mixer::delete_music (const SDL_mixer::Music& music)
{
  Mix_FreeMusic (music);
}

void SDL_mixer::delete_sound (const SDL_mixer::Sound& sound)
{
  Mix_FreeChunk (sound);
}

void SDL_mixer::start_music (const SDL_mixer::Music& music)
{
  Mix_PlayMusic (music, -1);
}

void SDL_mixer::pause_music (const SDL_mixer::Music&)
{
  Mix_VolumeMusic(10);
//  Mix_PauseMusic();
}

void SDL_mixer::resume_music (const SDL_mixer::Music&)
{
  Mix_VolumeMusic(64);
//  Mix_ResumeMusic();
}

void SDL_mixer::play_sound (const SDL_mixer::Sound& sound)
{
  Mix_PlayChannel(-1, sound, 0);
}


} // namespace Sosage::Third_party

#endif
