#pragma once

namespace quick {
namespace memory {

#ifndef QUICK_CACHE_LINE_SIZE
#define QUICK_CACHE_LINE_SIZE 64
#endif

#define QUICK_CACHE_LINE_ALIGNED alignas(QUICK_CACHE_LINE_SIZE)

}
}
