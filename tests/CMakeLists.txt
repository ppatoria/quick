cmake_minimum_required(VERSION 3.20)
project(tests)

message(STATUS "CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

 #################################################################
 #                     Dependencies                              #
 #################################################################

# -- Boost
#------------
find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found" )
endif()

# -- Catch2
#--------------
Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
  )
FetchContent_MakeAvailable(Catch2)

 ###################################################################
 #                      Unit Tests                                 #
 ###################################################################
add_executable( tests
  thread.cpp
  )

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

target_include_directories(tests PUBLIC
  ${CMAKE_SOURCE_DIR}/../../
  ${Boost_INCLUDE_DIR}
  )

  target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    # ${BOOST_LIBRARY_DIRS}/libboost_date_time.a
  )
enable_testing()
