#ifndef PN_CHAT_JSON_HPP
#define PN_CHAT_JSON_HPP

#include "nlohmann/json.hpp"
#include "string.hpp"
#include <optional>

class Json {
    public:
        class Iterator {
            public:
                Iterator(nlohmann::json::iterator it);
                Iterator operator++();
                bool operator!=(const Iterator& other) const;
                Json operator*() const;
                Json value() const;
                Pubnub::String key() const;
            private:
                nlohmann::json::iterator it;
        };

        Json() = default;
        Json(Pubnub::String json);
        Json(nlohmann::json json);
        static Json parse(Pubnub::String json);

        Json operator[](Pubnub::String key) const;
        Json operator[](int index) const;
        Json operator=(Pubnub::String json);
        operator Pubnub::String() const;
        
        bool contains(Pubnub::String key) const;
        bool is_null() const;
        Pubnub::String dump() const;
        std::optional<Pubnub::String> get_string(Pubnub::String key) const;

        void insert_or_update(Pubnub::String key, Json value);
        void insert_or_update(Pubnub::String key, Pubnub::String value);

        Iterator begin();
        Iterator end();
    private:
        nlohmann::json json;
};


#endif // PN_CHAT_JSON_HPP