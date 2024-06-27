#include <mutex>

template <typename T>
class MutexGuard {
public:
    MutexGuard(std::mutex& mutex, T& value);
    ~MutexGuard() = default;

    operator T&();
    T& operator*();
    T* operator->();
private:
    std::lock_guard<std::mutex> guard;
    T& value;
};

template <typename T>
class Mutex {
public:
    Mutex(T value);

    MutexGuard<T> lock();
private:
    T value;
    std::mutex mutex;
};
