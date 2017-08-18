#cmake_minimum_required(VERSION 3.1)
cmake_minimum_required(VERSION 2.8)
project(cnetpp)
add_definitions(-DUSE_SELECT)
#add_definitions(-DUSE_POLL)
#add_definitions(-DUSE_EVENTFD)
include_directories("${CMAKE_SOURCE_DIR}/src")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DLINUX_SYSTEM)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DDARWIN_SYSTEM)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DWINDOWS_SYSTEM)
else()
    add_definitions(-DUNIX_SYSTEM)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14")

file(GLOB HTTP_HEADER_FILES "src/cnetpp/http/*.h")
file(GLOB HTTP_SOURCE_FILES "src/cnetpp/http/*.cc")
file(GLOB TCP_HEADER_FILES "src/cnetpp/tcp/*.h")
file(GLOB TCP_SOURCE_FILES "src/cnetpp/tcp/*.cc")
file(GLOB BASE_HEADER_FILES "src/cnetpp/base/*.h")
file(GLOB BASE_SOURCE_FILES "src/cnetpp/base/*.cc")
file(GLOB CONCURRENCY_HEADER_FILES "src/cnetpp/concurrency/*.h")
file(GLOB CONCURRENCY_SOURCE_FILES "src/cnetpp/concurrency/*.cc")

set(SOURCE_FILES
    ${BASE_SOURCE_FILES}
    ${CONCURRENCY_SOURCE_FILES}
    ${HTTP_SOURCE_FILES}
    ${TCP_SOURCE_FILES})

# build shared library
add_library(cnetpp SHARED ${SOURCE_FILES})
set_target_properties(cnetpp PROPERTIES VERSION 1.0 SOVERSION 1)

# build static library
add_library(cnetpp_s STATIC ${SOURCE_FILES})
set_target_properties(cnetpp_s PROPERTIES OUTPUT_NAME "cnetpp")

link_directories("${PROJECT_BINARY_DIR}")
set(TEST_SERVER_SOURCE_FILES examples/server.cc)
add_executable(cnetpp_server_test ${TEST_SERVER_SOURCE_FILES})
target_link_libraries(cnetpp_server_test cnetpp pthread)

set(TEST_CLIENT_SOURCE_FILES examples/client.cc)
add_executable(cnetpp_client_test ${TEST_CLIENT_SOURCE_FILES})
target_link_libraries(cnetpp_client_test cnetpp pthread)

# Add unittests
add_subdirectory(third_party/gtest-1.7.0)
aux_source_directory(unittests UNITTEST_FILES)
add_executable(cnetpp_unittest ${UNITTEST_FILES})
target_include_directories(cnetpp_unittest PRIVATE third_party/gtest-1.7.0/include unittest)
target_link_libraries(cnetpp_unittest cnetpp gtest gtest_main pthread)

enable_testing()
add_test(unittest cnetpp_unittest)


install(TARGETS cnetpp cnetpp_s
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${BASE_HEADER_FILES} DESTINATION include/cnetpp/base)
install(FILES ${CONCURRENCY_HEADER_FILES} DESTINATION include/cnetpp/concurrency)
install(FILES ${HTTP_HEADER_FILES} DESTINATION include/cnetpp/http)
install(FILES ${TCP_HEADER_FILES} DESTINATION include/cnetpp/tcp)

