#ifndef SOSAGE_UTILS_TIME_H
#define SOSAGE_UTILS_TIME_H

#include <chrono>
#include <thread>

namespace Sosage
{

class Clock
{
  std::chrono::time_point<std::chrono::high_resolution_clock> m_latest;
  const int m_target_fps;
  const std::chrono::duration<double> m_refresh_time;
  std::chrono::high_resolution_clock m_clock;
  
public:

  Clock()
    : m_target_fps(50)
    , m_refresh_time (1. / double(m_target_fps))
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
      std::cerr << "\rFPS: " << 1. / (now - m_latest).count();
    m_latest = m_clock.now();
  }
  

};

} // namespace Sosage

#endif // SOSAGE_UTILS_TIME_H
