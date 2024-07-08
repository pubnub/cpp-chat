using System;
using System.Runtime.InteropServices;

namespace PubNubChatAPI.Entities
{
    public class MessageDraft : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_message_draft_delete(IntPtr message_draft);
        
        #endregion
        
        internal MessageDraft(IntPtr pointer) : base(pointer)
        {
        }

        protected override void DisposePointer()
        {
            pn_message_draft_delete(pointer);
        }
    }
}