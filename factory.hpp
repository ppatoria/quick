#include <boost/core/noncopyable.hpp>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <quick/phoenix.hpp>
#include <quick/thread/spin_lock.hpp>
#include <vector>

namespace quick {
/**
 * TEMPLATE factory Object.  OBJECT CREATION METHODS are registered
 * and invoked using a Key type (default: std::string).
 */

template <
    typename T,
    typename Key = std::string,
    typename DefaultCreator = std::function<std::unique_ptr<T>()>>
class factory : boost::noncopyable {
    template <typename U>
    static typename U::creator_type creator_type_(const typename U::creator_type*);

    template <typename U>
    static DefaultCreator creator_type_(...);

public:
    using creator = decltype(creator_type_<T>(nullptr));

    using creators = std::map<Key, creator>;
    using Keys = std::vector<Key>;

    /// Helper class used to automatically register and unregister
    /// factory methods
    class registrar : boost::noncopyable {
    public:
        registrar(const Key& key, const creator& func)
            : m_key(key)
        {
            factory::instance().insert(m_key, func);
        }

        ~registrar()
        {
            factory::instance().erase(m_key);
        }

        const Key& key() const { return m_key; }

    private:
        Key m_key;
    };

    /// Register a new factory method with the given key
    void insert(const Key& key, const creator& func)
    {
        Guard guard(_lock);

        erase(key);
        _creators[key] = func;
        _keys.push_back(key);
    }

    /// Erase the factory method with the matching key
    void erase(const Key& key)
    {
        Guard guard(_lock);

        if (1 == _creators.erase(key)) {
            typename Keys::iterator itr = _keys.begin();
            for (; itr != _keys.end(); ++itr) {
                if (key == *itr) {
                    _keys.erase(itr);
                    break;
                }
            }
        }
    }

    /// Invoke a factory method by its key.  Returns the result of the
    /// factory method if the key was found, else 0.
    decltype(auto) operator()(const Key& key)
    {
        return create(key);
    }

    /// Invoke a factory method by its key. Returns the result of the
    /// factory method if the key was found, else 0.
    template <typename... ARGS>
    decltype(auto) create(const Key& key, ARGS&&... args)
    {
        Guard guard(_lock);

        if (_creators.find(key) != _creators.end()) {
            return _creators[key](std::forward<ARGS>(args)...);
        }
        using Ret = decltype(std::declval<creator>()(std::forward<ARGS>(args)...));
        return Ret();
    }

    /// Invoke a factory method by its key. Returns the result of the
    /// factory method if the key was found, else 0.
    creator creator(const Key& key)
    {
        Guard guard(_lock);

        if (_creators.find(key) == _creators.end()) {
            std::ostringstream os;
            os << "Factory for key <" << key << "> is missing";
            throw std::invalid_argument(os.str());
        }

        return _creators[key];
    }

    bool exists(const Key& key) const
    {
        Guard guard(_lock);

        return _creators.find(key) != _creators.end();
    }

    /// Returns the list of valid keys in order of insertion.
    Keys keys() const
    {
        Guard guard(_lock);

        return _keys;
    }

    /// Access the singleton instance
    static factory& instance()
    {
        return phoenix<factory>::instance();
    }

private:
    mutable Lock _lock;
    Creators _creators;
    Keys _keys;
};
} // namespace quick
