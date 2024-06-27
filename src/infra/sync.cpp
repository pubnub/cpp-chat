#include "sync.hpp"

template <typename T>
MutexGuard<T>::MutexGuard(std::mutex& mutex, T& value) : guard(mutex), value(value) {}

template <typename T>
MutexGuard<T>::operator T&() {
    return value;
}

template <typename T>
T& MutexGuard<T>::operator*() {
    return value;
}

template <typename T>
T* MutexGuard<T>::operator->() {
    return &value;
}

template <typename T>
Mutex<T>::Mutex(T value) : value(value) {}

template <typename T>
MutexGuard<T> Mutex<T>::lock() {
    return MutexGuard<T>(mutex, value);
}
