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

        internal void UpdatePointer(IntPtr newPointer)
        {
            DisposePointer();
            pointer = newPointer;
        }

        internal abstract void UpdateWithPartialPtr(IntPtr partialPointer);

        protected abstract void DisposePointer();

        ~PointerWrapper()
        {
            DisposePointer();
        }
    }
}