#include "event_poller.h"
#include <tcp/epoll_event_poller_impl.h>
#include <tcp/select_event_poller_impl.h>
namespace cnetpp {
namespace tcp {

std::shared_ptr<EventPoller> EventPoller::New(size_t id,
                                              size_t max_connections) {
#if defined(USE_EPOLL)
  return std::shared_ptr<EventPoller>(
      new EpollEventPollerImpl(id, max_connections));
#elif defined(USE_SELECT)
  return std::shared_ptr<EventPoller>(
      new SelectEventPollerImpl(id, max_connections));
#endif
}

}  // namespace tcp
}  // namespace cnetpp
