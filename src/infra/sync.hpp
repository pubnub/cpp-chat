#ifndef PN_CHAT_SYNC_HPP
#define PN_CHAT_SYNC_HPP

#include <memory>
#include <mutex>

template <typename T>
class MutexGuard {
public:
    MutexGuard(std::mutex& mutex, T& value) : guard(mutex), value(value) {};
    ~MutexGuard() = default;

    operator T&() {
        return value;
    }

    T& operator*() {
        return value;
    }

    T* operator->() {
        return &value;
    }
private:
    std::lock_guard<std::mutex> guard;
    T& value;
};

template <typename T>
class Mutex {
public:
template<typename... Args>
    Mutex(Args&&... args): value(std::forward<Args>(args)...) {};
    ~Mutex() = default;

    MutexGuard<T> lock() {
        return MutexGuard<T>(mutex, value);
    };
private:
    T value;
    std::mutex mutex;
};

template <typename T>
using ThreadSafePtr = std::shared_ptr<Mutex<T>>;

#endif // PN_CHAT_SYNC_HPP
