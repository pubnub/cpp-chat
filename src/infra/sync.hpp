#ifndef PN_CHAT_SYNC_HPP
#define PN_CHAT_SYNC_HPP

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
    Mutex(T value): value(value) {};
    ~Mutex() = default;

    MutexGuard<T> lock() {
        return MutexGuard<T>(mutex, value);
    };
private:
    T value;
    std::mutex mutex;
};

#endif // PN_CHAT_SYNC_HPP
