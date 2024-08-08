using System;
using System.Collections.Generic;
using PubNubChatAPI.Entities;

namespace PubnubChatApi.Entities.Data
{
    public struct MembersResponseWrapper
    {
        public List<Membership> Memberships;
        public Page Page;
        public int Total;
        public string Status;
    }
    
    internal struct InternalMembersResponseWrapper
    {
        public IntPtr[] Memberships;
        public Page Page;
        public int Total;
        public string Status;
    }
}