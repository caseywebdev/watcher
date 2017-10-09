#ifndef WORKER_PLATFORM_H
#define WORKER_PLATFORM_H

#include <memory>
#include <string>
#include <utility>

#include "../errable.h"
#include "../message.h"
#include "../result.h"
#include "worker_thread.h"

class WorkerPlatform : public Errable
{
public:
  static std::unique_ptr<WorkerPlatform> for_worker(WorkerThread *thread);

  WorkerPlatform() : Errable("platform"){};
  virtual ~WorkerPlatform(){};

  virtual Result<> wake() = 0;

  virtual Result<> listen() = 0;
  virtual Result<bool> handle_add_command(const CommandID command,
    const ChannelID channel,
    const std::string &root_path) = 0;
  virtual Result<bool> handle_remove_command(const CommandID command, const ChannelID channel) = 0;

  Result<> handle_commands()
  {
    if (!is_healthy()) return health_err_result();

    Result<size_t> cr = thread->handle_commands();
    if (cr.is_error()) return cr.propagate();
    return ok_result();
  }

protected:
  WorkerPlatform(WorkerThread *thread) : Errable("platform"), thread{thread}
  {
    //
  }

  Result<> emit(Message &&message)
  {
    if (!is_healthy()) return health_err_result();

    return thread->emit(std::move(message));
  }

  template <class InputIt>
  Result<> emit_all(InputIt begin, InputIt end)
  {
    if (!is_healthy()) return health_err_result();

    return thread->emit_all(begin, end);
  }

  WorkerThread *thread;
};

#endif
