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
        static Json array();
        void push_back(Json&& val);

        Json operator[](Pubnub::String key) const;
        Json operator[](int index) const;
        Json operator=(Pubnub::String json);
        operator Pubnub::String() const;
        
        bool contains(Pubnub::String key) const;
        bool is_null() const;
        Pubnub::String dump() const;
        std::optional<Pubnub::String> get_string(Pubnub::String key) const;
        std::optional<bool> get_bool(Pubnub::String key) const;
        std::optional<int> get_int(Pubnub::String key) const;
        bool is_array() const;
        bool is_object() const;
        void erase(Pubnub::String key);

        void insert_or_update(Pubnub::String key, Json value);
        void insert_or_update(Pubnub::String key, Pubnub::String value);

        int as_int() const;

        Iterator begin();
        Iterator end();
    private:
        nlohmann::json json;
};


#endif // PN_CHAT_JSON_HPP
