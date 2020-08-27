/*
  [include/Sosage/Utils/thread.h]
  Interface for thread (or dummy if no thread allowed).

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#ifndef SOSAGE_UTILS_THREAD_H
#define SOSAGE_UTILS_THREAD_H

#ifdef SOSAGE_THREADS_ENABLED
#include <atomic>
#include <thread>
#endif


namespace Sosage
{

#ifdef SOSAGE_THREADS_ENABLED

class Thread
{
  std::atomic<Thread_state> m_thread_state = NO_THREAD;
  std::thread m_thread;

public:

  template <typename Function, typename... Args>
  void run (Function&& f, Args&&... args)
  {
    m_thread_state = STARTED;
    m_thread = std::thread (std::bind(f, args...));
  }

  void notify()
  {
    m_thread_state = FINISHED;
  }

  bool still_running()
  {
    if (m_thread_state == STARTED)
      return true;
    if (m_thread_state == FINISHED)
    {
      m_thread.join();
      m_thread_state = NO_THREAD;
      return false;
    }
    return false;
  }

};

#else

// If no thread allowed, this is just an empty shell that runs
// functions normally
class Thread
{
public:

  template <typename Function, typename... Args>
  void run (Function&& f, Args&&... args) { std::bind(f, args...)(); }
  void notify() { }
  bool still_running() { return false; }

};

#endif

}

#endif // SOSAGE_UTILS_THREAD_H
