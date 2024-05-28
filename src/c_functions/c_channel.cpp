#include "c_functions/c_channel.hpp"

void pn_channel_delete(Pubnub::Channel* channel) {
    delete channel;
}
