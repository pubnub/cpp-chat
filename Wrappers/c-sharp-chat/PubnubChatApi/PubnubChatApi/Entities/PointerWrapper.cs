using System;

namespace PubNubChatAPI.Entities
{
    public abstract class PointerWrapper
    {
        protected IntPtr pointer;
        internal IntPtr Pointer => pointer;
        
        public string Id { get; protected set; }

        internal PointerWrapper(IntPtr pointer, string uniqueId)
        {
            this.pointer = pointer;
            Id = uniqueId;
        }

        internal void UpdatePointer(IntPtr newPointer)
        {
            DisposePointer();
            pointer = newPointer;
        }

        protected abstract void DisposePointer();

        ~PointerWrapper()
        {
            DisposePointer();
        }
    }
}