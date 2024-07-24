#ifndef PN_CHAT_VECTOR_HPP
#define PN_CHAT_VECTOR_HPP

#include <cstddef>
#include <vector>

namespace Pubnub {

    template <typename T>
        class Vector {
            public:
                Vector() = default;

                Vector(std::initializer_list<T> list) {
                    reserve(list.size());
                    for (auto&& item : list) {
                        push_back(item);
                    }
                }

                Vector(std::vector<T>&& vec) {
                    reserve(vec.size());
                    for (auto&& item : vec) {
                        push_back(std::move(item));
                    }
                }

                Vector(const Vector& other) = default;

                Vector(Vector&& other) = default;

                Vector& operator=(const Vector& other) = default;

                Vector& operator=(Vector&& other) = default;

                ~Vector() {
                    delete[] data;
                }

                void push_back(const T& value) {
                    if (len == cap) {
                        reserve(calculate_capacity(len + 1));
                    }

                    data[len++] = value;
                }

                void reserve(std::size_t new_capacity) {
                    if (new_capacity <= cap) {
                        return;
                    }

                    T* new_data = new T[new_capacity];
                    for (std::size_t i = 0; i < len; ++i) {
                        new_data[i] = data[i];
                    }
                    delete[] data;
                    data = new_data;
                    cap = new_capacity;
                }

                T& operator[](std::size_t index) {
                    return data[index];
                }

                const T& operator[](std::size_t index) const {
                    return data[index];
                }

                std::size_t size() const {
                    return len;
                }

                std::size_t capacity() const {
                    return cap;
                }

                T* begin() {
                    return data;
                }

                T* end() {
                    return data + len;
                }

                const T* begin() const {
                    return data;
                }

                const T* end() const {
                    return data + len;
                }

                const T& front() const {
                    return data[0];
                }

                const T& back() const {
                    return data[len - 1];
                }

                T& front() {
                    return data[0];
                }

                T& back() {
                    return data[len - 1];
                }

                std::vector<T> into_std_vector() {
                    std::vector<T> std_vec(data, data + len);

                    data = nullptr;
                    len = 0;
                    cap = 0;

                    return std_vec;
                }
            private:
                std::size_t calculate_capacity(std::size_t len) const {
                    // For calculating the capacity we used the Rust's formula
                    // for the Vec's capacity calculation
                    // https://doc.rust-lang.org/1.61.0/src/alloc/raw_vec.rs.html#394
                    auto doubled_capacity = this->cap * 2;

                    return doubled_capacity > len ? doubled_capacity : len;
                }


                T* data = nullptr;
                std::size_t len = 0;
                std::size_t cap = 0;
        };
};

#endif // PN_CHAT_VECTOR_HPP
