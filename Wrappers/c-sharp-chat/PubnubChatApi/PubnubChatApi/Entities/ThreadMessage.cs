using System;
using System.Runtime.InteropServices;

namespace PubNubChatAPI.Entities
{
    public class ThreadMessage : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_thread_message_dispose(
            IntPtr thread_message);

        #endregion
        
        internal ThreadMessage(IntPtr pointer) : base(pointer)
        {
        }

        protected override void DisposePointer()
        {
            pn_thread_message_dispose(pointer);
        }
    }
}