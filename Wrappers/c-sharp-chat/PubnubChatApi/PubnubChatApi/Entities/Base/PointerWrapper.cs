using System;

namespace PubNubChatAPI.Entities
{
    public abstract class PointerWrapper
    {
        protected IntPtr pointer;
        internal IntPtr Pointer => pointer;
        
        internal PointerWrapper(IntPtr pointer)
        {
            this.pointer = pointer;
        }

        protected abstract void DisposePointer();

        ~PointerWrapper()
        {
            DisposePointer();
        }
    }
}