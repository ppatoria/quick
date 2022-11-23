#pragma once

#include <cstdlib>

namespace quick {

/**
 * A specialized Singleton wrapper that re-creates the object if it
 * is used after being destroyed.  Useful for very long-lived objects
 * that may be referred to after main() has finished (e.g. in static
 * dtors).  Inspired by a class of the same name from http://s11n.net
 */
template <typename T>
class phoenix : public T {
public:
    using self_type = phoenix<T>;

    static T& instance()
    {
        static self_type self;

        // If our dtor has been called, we need to re-construct ourselves
        // and register an atexit handler to cleanup later.
        if (_destroyed)
            new (&self) self_type;

        return self;
    }

private:
    phoenix() { _destroyed = false; }

    ~phoenix() { _destroyed = true; }

    static void atexit_handler()
    {
        if (!_destroyed) {
            static_cast<self_type&>(instance()).~phoenix();
        }
    }

    static bool _destroyed;
};

template <typename T>
bool phoenix<T>::_destroyed = false;

} // namespace quick
