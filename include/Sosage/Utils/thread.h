#ifndef SOSAGE_THREAD_H
#define SOSAGE_THREAD_H

#include <Sosage/Utils/time.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace Sosage
{

class Threadable
{
private:
  Clock m_clock;
  std::atomic<bool> m_running;

public:

  Threadable() : m_running(true) { }

  bool running() { return m_running; }
  void stop() { m_running = false; }
  void wait() { m_clock.wait(); }
};

template <typename T>
class Lockable : public T
{
private:
  std::mutex m_mutex;
public:

  void lock() { m_mutex.lock(); }
  void unlock() { m_mutex.unlock(); }
};

} // namespace Sosage

#endif // SOSAGE_THREAD_H
