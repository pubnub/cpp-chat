using System;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Enums;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class Channel : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_delete(IntPtr channel);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_connect(IntPtr channel, StringBuilder messages_json);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_disconnect(IntPtr channel);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_join(IntPtr channel, string additional_params);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_leave(IntPtr channel);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_set_restrictions(IntPtr channel, string user_id, bool ban_user,
            bool mute_user, string reason);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_send_text(IntPtr channel, string message, byte type, string metadata);

        #endregion
        
        private Chat chat;
        
        public event Action<Message> OnMessageReceived;
        public event Action<Channel> OnChannelUpdate;
        //TODO: wrap this further?
        public event Action<string> OnPresenceUpdate; 

        internal Channel(Chat chat, string channelId, IntPtr channelPointer) : base(channelPointer, channelId)
        {
            this.chat = chat;
        }
        
        internal static string GetChannelIdFromPtr(IntPtr userPointer)
        {
            //TODO: C++ getters ID
            return string.Empty;
        }

        //TODO: return actual Message objects
        public string Connect()
        {
            OnMessageReceived = null;
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_connect(pointer, messagesBuffer));
            return messagesBuffer.ToString();
        }

        public void Join()
        {
            OnMessageReceived = null;
            CUtilities.CheckCFunctionResult(pn_channel_join(pointer, string.Empty));
        }

        public void Disconnect()
        {
            CUtilities.CheckCFunctionResult(pn_channel_disconnect(pointer));
        }

        public void Leave()
        {
            CUtilities.CheckCFunctionResult(pn_channel_leave(pointer));
        }

        public void SetRestrictions(string userId, bool banUser, bool muteUser, string reason)
        {
            CUtilities.CheckCFunctionResult(pn_channel_set_restrictions(pointer, userId, banUser, muteUser,
                reason));
        }

        public void SendText(string message)
        {
            CUtilities.CheckCFunctionResult(pn_channel_send_text(pointer, message,
                (byte)pubnub_chat_message_type.PCMT_TEXT, string.Empty));
        }

        public void UpdateChannel(ChatChannelData updatedData)
        {
            chat.UpdateChannel(Id, updatedData);
        }

        public void DeleteChannel()
        {
            chat.DeleteChannel(Id);
        }

        protected override void DisposePointer()
        {
            pn_channel_delete(pointer);
        }
    }
}