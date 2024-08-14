using System;
using System.Runtime.InteropServices;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class MessageDraft : ChatEntity
    {
        #region DLL Imports

        [DllImport(DllImportSettings.DllName)]
        private static extern void pn_message_draft_delete(IntPtr message_draft);
        
        #endregion
        
        internal MessageDraft(IntPtr pointer) : base(pointer)
        {
        }
        
        internal override void UpdateWithPartialPtr(IntPtr partialPointer)
        {
            throw new NotImplementedException();
        }

        public override void StartListeningForUpdates()
        {
            throw new NotImplementedException();
        }

        protected override void DisposePointer()
        {
            pn_message_draft_delete(pointer);
        }
    }
}