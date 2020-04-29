#ifndef SOSAGE_CONFIG_H
#define SOSAGE_CONFIG_H

#include <Sosage/platform.h>
#include <memory>

namespace Sosage
{

#ifdef SOSAGE_WINDOWS
constexpr char folder_separator = '\\';
#else
constexpr char folder_separator = '/';
#endif

constexpr int world_width = 1920;
constexpr int world_height = 1000;
constexpr int world_depth = 3240;

constexpr int interface_depth = 1000000;
constexpr int inventory_back_depth = interface_depth + 1;
constexpr int inventory_front_depth = inventory_back_depth + 1;
constexpr int inventory_over_depth = inventory_front_depth + 1;
constexpr int cursor_depth = inventory_over_depth + 1;

constexpr int ground_map_width = 100;

constexpr int gui_fps = 60;
constexpr int animation_fps = 12;

constexpr int character_speed = 34;

constexpr int text_outline = 10;
constexpr int displayed_inventory_size = 4;

constexpr double button_click_duration = 0.1;


struct Config
{
  int interface_width;
  int interface_height;
  int window_width;
  int window_height;

  int characters_per_second;

  double ground_map_scaling;

  bool fullscreen;

  Config (int window_width = 1200, bool fullscreen = false)
    : interface_width (0)
    , interface_height (200)
    , window_width (window_width)
    , window_height (window_width * 10 / 16)
    , characters_per_second (12)
    , ground_map_scaling (world_width / double(ground_map_width))
    , fullscreen (fullscreen)
  { }

};

#ifdef SOSAGE_ANDROID
inline Config& config (int window_width = 1200, bool fullscreen = true)
#else
inline Config& config (int window_width = 1200, bool fullscreen = false)
#endif
{
  static std::unique_ptr<Config> c;
  if (c == std::unique_ptr<Config>())
    c = std::unique_ptr<Config>(new Config(window_width, fullscreen));
  return *c;
}

} // namespace Sosage

#endif // SOSAGE_CONFIG_H
