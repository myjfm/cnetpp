#cmake_minimum_required(VERSION 3.1)
cmake_minimum_required(VERSION 2.8)
project(cnetpp)

include_directories("${CMAKE_SOURCE_DIR}/src" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=c++11 -I./src/")


set(SOURCE_FILES
    src/concurrency/priority_queue.h
    src/concurrency/queue_base.cc
    src/concurrency/queue_base.h
    src/concurrency/rwlock.cc
    src/concurrency/rwlock.h
    src/concurrency/spin_lock.h
    src/concurrency/task.h
    src/concurrency/thread.cc
    src/concurrency/thread.h
    src/concurrency/thread_factory.cc
    src/concurrency/thread_factory.h
    src/concurrency/thread_manager.cc
    src/concurrency/thread_manager.h
    src/concurrency/thread_pool.h
    src/concurrency/thread_pool.cc
    src/http/http_packet.h
    src/http/http_packet.cc
    src/http/http_request.h
    src/http/http_request.cc
    src/http/http_response.cc
    src/http/http_response.h
    src/http/http_connection.h
    src/http/http_connection.cc
    src/http/http_callbacks.h
    src/http/http_options.h
    src/http/http_base.h
    src/http/http_base.cc
    src/http/http_client.h
    src/http/http_client.cc
    src/http/http_server.h
    src/http/http_server.cc
    src/tcp/command.h
    src/tcp/tcp_callbacks.h
    src/tcp/connection_base.h
    src/tcp/connection_factory.h
    src/tcp/connection_id.h
    src/tcp/connection_id.cc
    src/tcp/tcp_connection.h
    src/tcp/tcp_connection.cc
    src/tcp/listen_connection.h
    src/tcp/listen_connection.cc
    src/tcp/tcp_options.h
    src/tcp/epoll_event_poller_impl.cc
    src/tcp/epoll_event_poller_impl.h
    src/tcp/event.h
    src/tcp/event_center.h
    src/tcp/event_center.cc
    src/tcp/event_poller.h
    src/tcp/tcp_client.cc
    src/tcp/tcp_client.h
    src/tcp/tcp_server.h
    src/tcp/tcp_server.cc
    src/tcp/ring_buffer.h
    src/tcp/ring_buffer.cc
    src/base/end_point.cc
    src/base/end_point.h
    src/base/ip_address.cc
    src/base/ip_address.h
    src/base/socket.h
    src/base/socket.cc
    src/base/string_piece.cc
    src/base/string_piece.h
    src/base/string_utils.cc
    src/base/string_utils.h
    src/base/uri.cc
    src/base/uri.h
    src/base/csonpp.h
    src/base/csonpp_impl.h
    src/base/csonpp_impl.cc)

#add_executable(cnetpp ${SOURCE_FILES})
add_library(cnetpp SHARED ${SOURCE_FILES})

set(TEST_SERVER_SOURCE_FILES ${SOURCE_FILES} src/test/server.cc)
add_executable(cnetpp_server_test ${TEST_SERVER_SOURCE_FILES})

set(TEST_CLIENT_SOURCE_FILES ${SOURCE_FILES} src/test/client.cc)
add_executable(cnetpp_client_test ${TEST_CLIENT_SOURCE_FILES})

