using System.Collections.Generic;
using PubNubChatAPI.Entities;

namespace PubnubChatApi.Entities.Data
{
    public struct UsersResponseWrapper
    {
        public List<User> Users;
        public Page Page;
        public int Total;
    }
}