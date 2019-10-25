#ifndef SOSAGE_THREAD_H
#define SOSAGE_THREAD_H

#include <Sosage/Utils/time.h>

#include <atomic>
#include <thread>

namespace Sosage
{

class Threadable
{
private:
  std::thread m_thread;
  Clock m_clock;
  std::atomic<bool> m_running;

public:

  Threadable() : m_running(true) { }

  void start() { m_thread = std::thread([&]() { this->main(); }); }
  void join() { m_thread.join(); }
  bool running() { return m_running; }
  void stop() { m_running = false; }
  void wait() { m_clock.wait(); }
  virtual void main() = 0;
};

} // namespace Sosage

#endif // SOSAGE_THREAD_H
