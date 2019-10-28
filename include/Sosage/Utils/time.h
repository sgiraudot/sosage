#ifndef SOSAGE_UTILS_TIME_H
#define SOSAGE_UTILS_TIME_H

#include <Sosage/Config.h>

#include <chrono>
#include <thread>

namespace Sosage
{

class Clock
{
  std::chrono::time_point<std::chrono::high_resolution_clock> m_latest;
  const std::chrono::duration<double> m_refresh_time;
  std::chrono::high_resolution_clock m_clock;

  double m_mean;
  double m_active;
  std::size_t m_nb;

public:

  Clock()
    : m_refresh_time (1. / double(config().target_fps))
    , m_mean(0), m_active(0), m_nb(0)
  {
    m_latest = std::chrono::high_resolution_clock::now();
  }

  void wait(bool verbose)
  {
    std::chrono::time_point<std::chrono::high_resolution_clock>
      now = m_clock.now();
    
    std::chrono::duration<double> duration = now - m_latest;

    if (duration < m_refresh_time)
      std::this_thread::sleep_for(m_refresh_time - duration);

    now = m_clock.now();
    if (verbose)
    {
      m_active += (duration.count()) / double(m_refresh_time.count());
      m_mean += (now - m_latest).count();
      ++ m_nb;
      if (m_nb == 20)
      {
        std::cerr << "\rFPS: " << (m_nb / m_mean) * 1e9
                  << ", CPU: " << 100. * (m_active / m_nb) << "%";
        m_mean = 0.;
        m_active = 0.;
        m_nb = 0;
      }
    }
    m_latest = m_clock.now();
  }
  

};

} // namespace Sosage

#endif // SOSAGE_UTILS_TIME_H
