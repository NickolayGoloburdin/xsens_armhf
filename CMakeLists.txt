cmake_minimum_required(VERSION 3.0.0)
project(test_xsens VERSION 0.1.0)

include(CTest)
enable_testing()
set(THREADS_PREFER_PTHREAD_FLAG ON)
include_directories(test_xsens PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_executable(test_xsens main.cpp src/xsensereciever.cpp )


find_package(Threads REQUIRED
xsens REQUIRED)
target_link_libraries(test_xsens PUBLIC Threads::Threads xsens )
#target_link_libraries(test_xsens PUBLIC xsens xscommon)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
