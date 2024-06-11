using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Enums;

namespace PubNubChatAPI.Entities
{
    public class Channel
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_delete(IntPtr channel);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_connect(IntPtr channel, StringBuilder messages_json);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_disconnect(IntPtr channel);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_join(IntPtr channel, string additional_params);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_leave(IntPtr channel);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_set_restrictions(IntPtr channel, string user_id, bool ban_user, bool mute_user, string reason);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_send_text(IntPtr channel, string message, byte type, string metadata);
        #endregion
        
        private IntPtr channelPointer;
        private Chat chat;
        
        public string ChannelId { get; }
        public event Action<Message> OnMessageReceived;
        
        internal Channel(Chat chat, string channelId, IntPtr channelPointer)
        {
            this.chat = chat;
            ChannelId = channelId;
            this.channelPointer = channelPointer;
        }

        //TODO: return actual Message objects
        public string Connect()
        {
            OnMessageReceived = null;
            var messagesBuffer = new StringBuilder(32768);
            pn_channel_connect(channelPointer, messagesBuffer);
            return messagesBuffer.ToString();
        }
        
        public void Join()
        {
            OnMessageReceived = null;
            pn_channel_join(channelPointer, string.Empty);
        }

        public void Disconnect()
        {
            pn_channel_disconnect(channelPointer);
        }

        public void Leave()
        {
            pn_channel_leave(channelPointer);
        }

        public void SetRestrictions(string userId, bool banUser, bool muteUser, string reason)
        {
            pn_channel_set_restrictions(channelPointer, userId, banUser, muteUser, reason);
        }

        public void SendText(string message)
        {
            pn_channel_send_text(channelPointer, message, (byte)pubnub_chat_message_type.PCMT_TEXT, string.Empty);
        }

        public void UpdateChannel(ChatChannelData updatedData)
        {
            chat.UpdateChannel(ChannelId, updatedData);
        }

        public void DeleteChannel()
        {
            chat.DeleteChannel(ChannelId);
        }

        ~Channel()
        {
            pn_channel_delete(channelPointer);
        }
    }
}