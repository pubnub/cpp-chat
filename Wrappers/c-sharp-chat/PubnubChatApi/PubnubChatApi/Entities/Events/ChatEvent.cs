using System;
using PubNubChatAPI.Entities;

namespace PubnubChatApi.Entities.Events
{
    public abstract class ChatEvent : PointerWrapper
    {
        //TODO: temporary, waiting for C++ 
        public string Json { get; }
        
        internal ChatEvent(IntPtr pointer, string json) : base(pointer)
        {
            Json = json;
        }
    }
}