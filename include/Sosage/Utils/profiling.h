#ifndef SOSAGE_UTILS_PROFILING_H
#define SOSAGE_UTILS_PROFILING_H

#include <Sosage/Core/Time.h>

namespace Sosage
{
using namespace Core;

class Timer
{
  std::string m_id;
  Time::Unit m_start;
  Time::Duration m_duration;
  unsigned int m_nb;
  
public:

  Timer (const std::string& id) : m_id (id), m_nb(0) { }

  ~Timer()
  {
    output("[" + m_id + " profiling] " + to_string(m_duration) + " ("
           + to_string(mean_duration()) + " per iteration)");
  }

  void start()
  {
    m_start = Time::now();
    ++ m_nb;
  }


  void stop()
  {
    m_duration += Time::now() - m_start;
  }

  double mean_duration() const { return m_duration / double(m_nb); }

  std::string to_string (double d)
  {
    if (d < 100)
      return std::to_string(d) + "ms";
    return std::to_string(d / 1000.) + "s";
  }
};


}


#endif // SOSAGE_UTILS_PROFILING_H


