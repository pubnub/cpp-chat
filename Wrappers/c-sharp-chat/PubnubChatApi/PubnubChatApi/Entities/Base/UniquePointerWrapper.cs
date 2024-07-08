using System;

namespace PubNubChatAPI.Entities
{
    public abstract class UniquePointerWrapper : PointerWrapper
    {
        public string Id { get; protected set; }

        protected UniquePointerWrapper(IntPtr pointer, string uniqueId) : base(pointer)
        {
            Id = uniqueId;
        }
    }
}