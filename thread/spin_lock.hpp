#pragma once

#include <quick/memory/align.hpp>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>

namespace quick {
namespace thread {

    inline void pause()
    {
#if defined(__GNUG__)
        asm volatile("pause");
#elif defined(_MSC_VER)
        _mm_pause();
#endif
    }

    /*
       non-recursive
       implements Lockable concept, can be used with std::lock_guard<>
       neither copyable nor movable due to _lock's type
    */

    class spin_lock {
    public:
        using guard = std::lock_guard<spin_lock>;

    public:
        spin_lock()
            : _lock(0)
        {
        }

    public:
        void lock()
        {
            while (!try_lock()) {
                while (_lock.load(std::memory_order_acquire)) {
                    pause();
                }
            }
        }

        void unlock()
        {
            _lock.store(0, std::memory_order_release);
        }

        bool try_lock()
        {
            return 0 == _lock.exchange(1, std::memory_order_seq_cst);
        }

    public: // for compatibility with old interface
        bool tryLock() { return try_lock(); }
        int acquire()
        {
            lock();
            return 1;
        }
        int release()
        {
            unlock();
            return 0;
        }

    protected:
        QUICK_CACHE_LINE_ALIGNED
        std::atomic<int> _lock;
    };

    /*
       recursive, based on thread ownership
       implements Lockable concept, can be used with std::lock_guard<>
       neither copyable nor movable due to _lock's type
    */

    class recursive_spin_lock {
    public:
        using guard = std::lock_guard<recursive_spin_lock>;

    public:
        recursive_spin_lock()
            : _lock(0)
            , _count(0)
        {
        }

    public:
        void lock()
        {
            while (!try_lock()) {
                while (_lock.load(std::memory_order_acquire)) {
                    pause();
                }
            }
        }

        void unlock()
        {
            if (std::this_thread::get_id() == _owner) {
                if (0 == --_count) {
                    _owner = std::thread::id();
                    _lock.store(0, std::memory_order_release);
                }
            }
        }

        bool try_lock()
        {
            if (std::this_thread::get_id() == _owner) {
                ++_count;
                return true;
            } else {
                if (0 == _lock.exchange(1, std::memory_order_seq_cst)) {
                    _owner = std::this_thread::get_id();
                    ++_count;
                    return true;
                }
            }
            return false;
        }

    public: // for compatibility with old interface
        bool tryLock() { return try_lock(); }
        int acquire()
        {
            lock();
            return 1;
        }
        int release()
        {
            unlock();
            return 0;
        }

        std::uint64_t recursionLevel() const { return _count; }

    private:
        QUICK_CACHE_LINE_ALIGNED
        std::atomic<int> _lock;
        std::uint64_t _count;
        std::thread::id _owner;
    };
} // namespace memory
} // namespace quick
