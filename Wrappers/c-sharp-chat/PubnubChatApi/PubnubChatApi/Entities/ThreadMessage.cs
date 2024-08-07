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

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_thread_message_unpin_from_parent_channel(IntPtr thread_message);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_thread_message_pin_to_parent_channel(IntPtr thread_message);

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

        public void PinMessageToParentChannel()
        {
            var newChannelPointer = pn_thread_message_pin_to_parent_channel(pointer);
            CUtilities.CheckCFunctionResult(newChannelPointer);
            //TODO: this is to update the pointer of the existing wrapper, but isn't very explicit about the fact it does that
            chat.TryGetChannel(newChannelPointer, out _);
        }

        public void UnPinMessageFromParentChannel()
        {
            var newChannelPointer = pn_thread_message_unpin_from_parent_channel(pointer);
            CUtilities.CheckCFunctionResult(newChannelPointer);
            //TODO: this is to update the pointer of the existing wrapper, but isn't very explicit about the fact it does that
            chat.TryGetChannel(newChannelPointer, out _);
        }

        protected override void DisposePointer()
        {
            pn_thread_message_dispose(pointer);
        }
    }
}