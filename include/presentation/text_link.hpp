#ifndef PN_TEXT_LINK_H
#define PN_TEXT_LINK_H

#include "string.hpp"

namespace Pubnub
{
    struct TextLink
    {
        int start_index = 0;
        int end_index = 0;
        Pubnub::String link = "";
    };
}
#endif /* PN_TEXT_LINK_H */
