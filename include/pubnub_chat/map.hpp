#ifndef PN_CHAT_MAP_HPP
#define PN_CHAT_MAP_HPP

#include "vector.hpp"
#include <map>
#include <iostream>

namespace Pubnub {

    template <typename T, typename K>
    struct Map
    {
        Map(){};
        Map(std::map<T, K> in_map) {
            for(auto it = in_map.begin(); it != in_map.end(); it++)
            {
                keys.push_back(it->first);
                values.push_back(it->second);
            }
        }

        Pubnub::Vector<T> keys;
        Pubnub::Vector<K> values;

        std::map<T, K> into_std_map(){
            if(keys.size() != values.size())
            {
                throw std::runtime_error("Map is broken, can't convert into std::map");
            }

            std::vector<T> std_keys = keys.into_std_vector();
            std::vector<K> std_values = values.into_std_vector();

            std::map<T, K> final_map;
            for(int i = 0; i < std_keys.size(); i++)
            {
                final_map[std_keys[i]] = std_values[i];
            }

            return final_map;
        }

    };
}
#endif // PN_CHAT_VECTOR_HPP
