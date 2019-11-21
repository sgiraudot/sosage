#ifndef SOSAGE_CONFIG_H
#define SOSAGE_CONFIG_H

#include <Sosage/platform.h>
#include <memory>

namespace Sosage
{

struct Config
{
  const int world_width;
  const int world_height;
  const int world_depth;
  const int ground_map_width;
  const int gui_fps;
  const int animation_fps;
  const int character_speed;
  
  int camera_width;
  int camera_height;
  double camera_scaling;
  double ground_map_scaling;

  bool fullscreen;

  Config (int camera_width = 1200, bool fullscreen = false)
    : world_width (1920)
    , world_height (1080)
    , world_depth (3240)
    , ground_map_width (150)
    , gui_fps (30)
    , animation_fps (12)
    , character_speed (340 / animation_fps)
    , camera_width (camera_width)
    , camera_height (camera_width * world_height / world_width)
    , camera_scaling (world_width / double(camera_width))
    , ground_map_scaling (world_width / double(ground_map_width))
    , fullscreen (fullscreen)
  { }

};

#ifdef SOSAGE_ANDROID
inline Config& config (int camera_width = 1200, bool fullscreen = true)
#else
inline Config& config (int camera_width = 1200, bool fullscreen = false)
#endif
{
  static std::unique_ptr<Config> c;
  if (c == std::unique_ptr<Config>())
    c = std::unique_ptr<Config>(new Config(camera_width, fullscreen));
  return *c;
}

} // namespace Sosage

#endif // SOSAGE_CONFIG_H
