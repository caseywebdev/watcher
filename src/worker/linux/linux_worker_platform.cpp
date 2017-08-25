#include <string>
#include <memory>
#include <poll.h>

#include "../worker_platform.h"
#include "../worker_thread.h"
#include "../../message.h"
#include "../../log.h"
#include "../../result.h"
#include "../../helper/linux/helper.h"
#include "pipe.h"

using std::string;
using std::unique_ptr;

class LinuxWorkerPlatform : public WorkerPlatform {
public:
  LinuxWorkerPlatform(WorkerThread *thread) :
    WorkerPlatform(thread),
    pipe("worker pipe")
  {
    //
  };

  Result<> wake() override
  {
    return pipe.signal();
  }

  Result<> listen() override
  {
    pollfd to_poll[1];
    to_poll[0].fd = pipe.get_read_fd();
    to_poll[0].events = POLLIN;
    to_poll[0].revents = 0;

    while (true) {
      int result = poll(to_poll, 1, -1);

      if (result < 0) {
        return errno_result<>("Unable to poll");
      } else if (result == 0) {
        return error_result("Unexpected poll() timeout");
      }

      if (to_poll[0].revents & (POLLIN | POLLERR)) {
        Result<> hr = handle_commands();
        if (hr.is_error()) return hr;

        Result<> cr = pipe.consume();
        if (cr.is_error()) return cr;
      }
    }

    return error_result("Polling loop exited unexpectedly");
  }

  Result<bool> handle_add_command(
    const CommandID command,
    const ChannelID channel,
    const string &root_path) override
  {
    return ok_result(true);
  }

  Result<bool> handle_remove_command(
    const CommandID command,
    const ChannelID channel) override
  {
    return ok_result(true);
  }

private:
  Pipe pipe;
};

unique_ptr<WorkerPlatform> WorkerPlatform::for_worker(WorkerThread *thread)
{
  return unique_ptr<WorkerPlatform>(new LinuxWorkerPlatform(thread));
}
