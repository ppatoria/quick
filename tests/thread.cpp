#include "catch2/catch_all.hpp"
#include "quick/phoenix.hpp"
#include "quick/thread/spin_lock.hpp"
#include <cstdint>
#include <string>

TEST_CASE("thread", "[misc tests]")
{    
    quick::thread::spin_lock lock;
}
