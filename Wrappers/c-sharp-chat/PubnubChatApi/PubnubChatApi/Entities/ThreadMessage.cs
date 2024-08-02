using System;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class ThreadMessage : Message
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_thread_message_dispose(
            IntPtr thread_message);

        [DllImport("pubnub-chat")]
        private static extern int pn_thread_message_get_timetoken(IntPtr thread_message, StringBuilder result);

        #endregion

        internal ThreadMessage(Chat chat, IntPtr messagePointer, string timeToken) : base(chat, messagePointer,
            timeToken)
        {
        }

        internal static string GetThreadMessageIdFromPtr(IntPtr threadMessagePointer)
        {
            var buffer = new StringBuilder(128);
            CUtilities.CheckCFunctionResult(pn_thread_message_get_timetoken(threadMessagePointer, buffer));
            return buffer.ToString();
        }

        protected override void DisposePointer()
        {
            pn_thread_message_dispose(pointer);
        }
    }
}