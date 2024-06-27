#ifndef PN_CHAT_REPOSITORY_HPP
#define PN_CHAT_REPOSITORY_HPP

#include <map>
#include <utility>
#include "sync.hpp"

template <typename Key, typename Value>
class Repository {
    public:
        Repository(): data(std::map<Key, Value>()) {}
        ~Repository() = default;

        void update_or_insert(Key key, Value value) {
            auto guard = data.lock();
            guard->insert_or_assign(key, value);
        };

        void update_or_insert(std::pair<Key, Value> pair) {
            auto guard = data.lock();
            guard->insert_or_assign(pair.first, pair.second);
        };

        void remove(Key key) {
            auto guard = data.lock();
            guard->erase(key);
        };

        bool exists(Key key) {
            auto guard = data.lock();
            return guard->find(key) != guard->end();
        };

        Value get(Key key) {
            auto guard = data.lock();
            return guard->at(key);
        };

    private:
        Mutex<std::map<Key, Value>> data;
};

#endif // PN_CHAT_REPOSITORY_HPP
