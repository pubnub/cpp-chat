using PubNubChatAPI.Entities;

namespace PubnubChatApi.Entities.Data
{
    public struct UnreadMessageWrapper
    {
        public Channel Channel;
        public Membership Membership;
        public int Count;
    }
}