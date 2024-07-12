#ifndef PN_CHAT_REPOSITORY_HPP
#define PN_CHAT_REPOSITORY_HPP

#include <map>
#include <optional>
#include <utility>
#include "sync.hpp"

template <typename Key, typename Value, typename Compare = std::less<Key>>
class Repository {
    public:
        Repository(): data(std::map<Key, Value, Compare>()) {}
        ~Repository() = default;

        void update_or_insert(Key key, Value value) {
            auto guard = this->data.lock();
            guard->insert_or_assign(key, value);
        };

        void update_or_insert(std::pair<Key, Value> pair) {
            auto guard = this->data.lock();
            guard->insert_or_assign(pair.first, pair.second);
        };

        void remove(Key key) {
            auto guard = this->data.lock();
            guard->erase(key);
        };

        bool exists(Key key) {
            auto guard = this->data.lock();
            return this->exists(key, *guard);
        };

        std::optional<Value> get(Key key) {
            auto guard = this->data.lock();
            if (this->exists(key, *guard)) {
                return guard->at(key);
            }

            return {};
        };

    private:
        Mutex<std::map<Key, Value, Compare>> data;

        bool exists(Key key, std::map<Key, Value, Compare> data) {
            return data.find(key) != data.end();
        };
};

#endif // PN_CHAT_REPOSITORY_HPP
