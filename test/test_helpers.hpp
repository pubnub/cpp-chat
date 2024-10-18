#include "string.hpp"

Pubnub::String trim_whitespaces(Pubnub::String to_trim) {
    for (int i = 0; i < to_trim.length(); i++) {
        if (to_trim[i] == ' ') {
            to_trim.erase(i, 1);
            i--;
        }
    }

    return to_trim;
}

template <typename T>
bool vector_contains(const std::vector<T>& vec, const T& value) {
    return std::find_if(
            vec.begin(),
            vec.end(),
            [&value](auto other) {return value == other;}
        ) != vec.end();
}


