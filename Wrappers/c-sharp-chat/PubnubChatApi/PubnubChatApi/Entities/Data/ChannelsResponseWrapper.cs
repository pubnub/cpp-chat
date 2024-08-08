using System.Collections.Generic;
using PubNubChatAPI.Entities;

namespace PubnubChatApi.Entities.Data
{
    public struct ChannelsResponseWrapper
    {
        public List<Channel> Channels;
        public Page Page;
        public int Total;
    }
}