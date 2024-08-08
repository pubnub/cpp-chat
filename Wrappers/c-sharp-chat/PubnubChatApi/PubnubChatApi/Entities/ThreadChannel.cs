using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class ThreadChannel : Channel
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_thread_channel_dispose(
            IntPtr thread_channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_thread_channel_get_history(
            IntPtr thread_channel,
            string start_timetoken,
            string end_timetoken,
            int count,
            StringBuilder thread_messages_pointers_json);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_thread_channel_pin_message_to_parent_channel(IntPtr thread_channel,
            IntPtr message);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_thread_channel_unpin_message_from_parent_channel(IntPtr thread_channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_thread_channel_send_text(IntPtr thread_channel, string text);
        
        [DllImport("pubnub-chat")]
        private static extern int pn_thread_channel_get_parent_channel_id(IntPtr thread_channel, StringBuilder result);
        
        [DllImport("pubnub-chat")]
        private static extern int pn_thread_channel_emit_user_mention(IntPtr thread_channel, string user_id, string timetoken, string text);
            
        #endregion

        public string ParentChannelId
        {
            get
            {
                var buffer = new StringBuilder(128);
                CUtilities.CheckCFunctionResult(pn_thread_channel_get_parent_channel_id(pointer, buffer));
                return buffer.ToString();
            }
        }
        
        internal static string MessageToThreadChannelId(Message message)
        {
            return $"PUBNUB_INTERNAL_THREAD_{message.ChannelId}_{message.Id}";
        }

        internal ThreadChannel(Chat chat, Message sourceMessage, IntPtr channelPointer) : base(chat,
            MessageToThreadChannelId(sourceMessage),
            channelPointer)
        {
        }

        public List<ThreadMessage> GetThreadHistory(string startTimeToken, string endTimeToken, int count)
        {
            var buffer = new StringBuilder(4096);
            CUtilities.CheckCFunctionResult(pn_thread_channel_get_history(pointer, startTimeToken, endTimeToken, count,
                buffer));
            var messagesPointersJson = buffer.ToString();
            var history = new List<ThreadMessage>();
            if (!CUtilities.IsValidJson(messagesPointersJson))
            {
                return history;
            }

            var messagePointers = JsonConvert.DeserializeObject<IntPtr[]>(messagesPointersJson);
            if (messagePointers == null)
            {
                return history;
            }

            foreach (var messagePointer in messagePointers)
            {
                var id = ThreadMessage.GetThreadMessageIdFromPtr(messagePointer);
                history.Add(new ThreadMessage(chat, messagePointer, id));
            }

            return history;
        }

        public override void EmitUserMention(string userId, string timeToken, string text)
        {
            CUtilities.CheckCFunctionResult(pn_thread_channel_emit_user_mention(pointer, userId, timeToken, text));
        }

        public override void SendText(string message)
        {
            CUtilities.CheckCFunctionResult(pn_thread_channel_send_text(pointer, message));
        }

        public void PinMessageToParentChannel(Message message)
        {
            var newChannelPointer = pn_thread_channel_pin_message_to_parent_channel(pointer, message.Pointer);
            CUtilities.CheckCFunctionResult(newChannelPointer);
            //TODO: this is to update the pointer of the existing wrapper, but isn't very explicit about the fact it does that
            chat.TryGetChannel(ParentChannelId, newChannelPointer, out _);
        }

        public void UnPinMessageFromParentChannel()
        {
            var newChannelPointer = pn_thread_channel_unpin_message_from_parent_channel(pointer);
            CUtilities.CheckCFunctionResult(newChannelPointer);
            //TODO: this is to update the pointer of the existing wrapper, but isn't very explicit about the fact it does that
            chat.TryGetChannel(ParentChannelId, newChannelPointer, out _);
        }

        protected override void DisposePointer()
        {
            pn_thread_channel_dispose(pointer);
        }
    }
}