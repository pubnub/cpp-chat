#ifndef PN_CHAT_JSON_HPP
#define PN_CHAT_JSON_HPP

#include "nlohmann/json.hpp"
#include "string.hpp"

class Json {
    public:
        Json(Pubnub::String json);
        Json(nlohmann::json json);
        static Json parse(Pubnub::String json);

        Json operator[](Pubnub::String key);
        Json operator[](int index);
        operator Pubnub::String() const;
        bool contains(Pubnub::String key);
    private:
        nlohmann::json json;
};


#endif // PN_CHAT_JSON_HPP
