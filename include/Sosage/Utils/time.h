#ifndef SOSAGE_UTILS_TIME_H
#define SOSAGE_UTILS_TIME_H

#include <Sosage/Config.h>

#include <Sosage/Core/Time.h>

namespace Sosage
{
using namespace Core;


class Clock
{
  Time::Unit m_latest;
  Time::Duration m_gui_refresh_time;
  Time::Duration m_animation_refresh_time;

  double m_mean;
  double m_active;
  std::size_t m_nb;
  
  double m_fps;
  double m_cpu;

  Time::Unit m_start;
  std::size_t m_frame_id;

  Clock (const Clock&) { }

public:

  Clock()
    : m_gui_refresh_time (1000. / double(config().gui_fps))
    , m_animation_refresh_time (1000. / double(config().animation_fps))
    , m_mean(0), m_active(0), m_nb(0), m_fps(config().gui_fps), m_cpu(0)
    , m_frame_id(0)
  {
    m_start = Time::now();
    m_latest = m_start;
  }

  void wait(bool verbose)
  {
    Uint32 now = Time::now();
    Uint32 duration = now - m_latest;

    if (duration < m_gui_refresh_time)
      Time::wait (m_gui_refresh_time - duration);

    now = Time::now();
    if (verbose)
    {
      m_active += duration;
      m_mean += (now - m_latest);
      ++ m_nb;
      if (m_nb == 20)
      {
        m_fps = 1000. / (m_mean / m_nb);
        m_cpu = 100. * (m_active / (m_nb * m_gui_refresh_time));
        m_mean = 0.;
        m_active = 0.;
        m_nb = 0;
      }
    }
    m_latest = now;

    m_frame_id = std::size_t((m_latest - m_start) / m_animation_refresh_time);
  }

  double fps() const { return m_fps; }
  double cpu() const { return m_cpu; }

  std::size_t frame_id() const { return m_frame_id; }
  std::size_t frame_time() const { return m_frame_id / config().animation_fps; }

};

} // namespace Sosage

#endif // SOSAGE_UTILS_TIME_H
