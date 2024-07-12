#ifndef PN_RESTRICTIONS_H
#define PN_RESTRICTIONS_H

#include "string.hpp"

namespace Pubnub
{
    struct Restriction
    {
        bool ban = false;
        bool mute = false;
        Pubnub::String reason = "";
    };
}
#endif /* PN_RESTRICTIONS_H */
