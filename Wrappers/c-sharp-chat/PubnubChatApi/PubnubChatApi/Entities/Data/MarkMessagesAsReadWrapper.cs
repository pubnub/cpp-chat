using System.Collections.Generic;
using PubNubChatAPI.Entities;

namespace PubnubChatApi.Entities.Data
{
    public struct MarkMessagesAsReadWrapper
    {
        public Page Page;
        public int Total;
        public int Status;
        public List<Membership> Memberships;
    }
}