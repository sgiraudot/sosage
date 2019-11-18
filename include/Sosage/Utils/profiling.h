#ifndef SOSAGE_UTILS_PROFILING_H
#define SOSAGE_UTILS_PROFILING_H

#include <Sosage/Core/Time.h>

namespace Sosage
{
using namespace Core;

class Timer
{
  Time::Unit m_start;
  Time::Duration m_duration;
  
public:

  Timer() { }

  void start()
  {
    m_start = Time::now();
  }


  void stop()
  {
    m_duration += Time::now() - m_start;
  }

  double duration() const { return m_duration / 1000.; }

};

class Profiling
{
  std::vector<std::pair<std::string, Timer> > m_timers;

public:

  Profiling() { }

  ~Profiling()
  {
    debug("Profiling results:");

    double total = 0;
    for (const auto& t : m_timers)
      total += t.second.duration();

    for (const auto& t : m_timers)
      debug(" * " + t.first + ": " +  std::to_string(t.second.duration()) + "s ("
            + std::to_string(100. * t.second.duration() / total) + "%)");
  }

  std::size_t init(const std::string& name)
  {
    m_timers.push_back (std::make_pair (name, Timer()));
    return m_timers.size() - 1;
  }

  void start (std::size_t idx) { m_timers[idx].second.start(); }
  void stop (std::size_t idx) { m_timers[idx].second.stop(); }

};

}


#endif // SOSAGE_UTILS_PROFILING_H


