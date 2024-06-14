using System;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class Message : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_message_delete(IntPtr message);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_message_edit_text(IntPtr message, string text);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_message_text(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_message_delete_message(IntPtr message);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_message_deleted(IntPtr message);
        
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_message_get_timetoken(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_message_get_data_type(IntPtr message);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_message_get_data_text(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_message_get_data_channel_id(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_message_get_data_user_id(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_message_get_data_meta(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_message_get_data_message_actions(IntPtr message, StringBuilder result);

        #endregion

        public string MessageText
        {
            get
            {
                var buffer = new StringBuilder(32768);
                CUtilities.CheckCFunctionResult(pn_message_text(pointer, buffer));
                return buffer.ToString();
            }
        }
        public string TimeToken
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_message_get_timetoken(pointer, buffer);
                return buffer.ToString();
            }
        }
        public string ChannelId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_message_get_data_channel_id(pointer, buffer);
                return buffer.ToString();
            }
        }
        public string UserId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_message_get_data_user_id(pointer, buffer);
                return buffer.ToString();
            }
        }
        public string Meta
        {
            get
            {
                var buffer = new StringBuilder(4096);
                pn_message_get_data_meta(pointer, buffer);
                return buffer.ToString();
            }
        }
        public bool IsDeleted
        {
            get
            {
                var result = pn_message_deleted(pointer);
                CUtilities.CheckCFunctionResult(result);
                return result == 1;
            }
        }
        
        private Chat chat;
        public event Action<Message> OnMessageUpdated; 
        
        internal Message(Chat chat, IntPtr messagePointer, string timeToken) : base(messagePointer, timeToken)
        {
            this.chat = chat;
        }
        
        internal static string GetMessageIdFromPtr(IntPtr messagePointer)
        {
            var buffer = new StringBuilder(512);
            pn_message_get_timetoken(messagePointer, buffer);
            return buffer.ToString();
        }
        
        internal static string GetChannelIdFromMessagePtr(IntPtr messagePointer)
        {
            var buffer = new StringBuilder(512);
            pn_message_get_data_channel_id(messagePointer, buffer);
            return buffer.ToString();
        }

        internal void BroadcastMessageUpdate()
        {
            OnMessageUpdated?.Invoke(this);
        }

        public void EditMessageText(string newText)
        {
            CUtilities.CheckCFunctionResult(pn_message_edit_text(pointer, newText));
        }

        public void DeleteMessage()
        {
            CUtilities.CheckCFunctionResult(pn_message_delete_message(pointer));
        }

        protected override void DisposePointer()
        {
            pn_message_delete(pointer);
        }
    }
}