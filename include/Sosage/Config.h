#ifndef SOSAGE_CONFIG_H
#define SOSAGE_CONFIG_H

#include <memory>

namespace Sosage
{

struct Config
{
  const int world_width;
  const int world_height;
  const int ground_map_width;
  const int ground_map_height;
  const int character_speed;
  
  int camera_width;
  int camera_height;
  double camera_scaling;
  double ground_map_scaling;

  bool fullscreen;

  Config (int camera_width = 1200, bool fullscreen = false)
    : world_width (1920)
    , world_height (1080)
    , ground_map_width (300)
    , ground_map_height (ground_map_height * world_height / world_width)
    , character_speed (5)
    , camera_width (camera_width)
    , camera_height (camera_width * world_height / world_width)
    , camera_scaling (world_width / double(camera_width))
    , ground_map_scaling (world_width / double(ground_map_width))
    , fullscreen (fullscreen)
  { }

};

inline Config& config (int camera_width = 1200, bool fullscreen = false)
{
  static std::unique_ptr<Config> c;
  if (c == std::unique_ptr<Config>())
    c = std::unique_ptr<Config>(new Config(camera_width, fullscreen));
  return *c;
}

} // namespace Sosage

#endif // SOSAGE_CONFIG_H
