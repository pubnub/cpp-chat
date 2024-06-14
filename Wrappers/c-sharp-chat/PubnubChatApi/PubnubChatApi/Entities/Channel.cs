using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
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
        private static extern int pn_channel_join(IntPtr channel, string additional_params, StringBuilder messages_json);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_leave(IntPtr channel);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_set_restrictions(IntPtr channel, string user_id, bool ban_user,
            bool mute_user, string reason);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_channel_id(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_send_text(IntPtr channel, string message, byte type, string metadata);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_get_user_restrictions(
            IntPtr channel,
            string user_id,
            string channel_id,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_data_channel_name(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_data_description(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_data_custom_data_json(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_data_updated(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_data_status(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_channel_get_data_type(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_is_present(IntPtr channel, string user_id);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_who_is_present(IntPtr channel, StringBuilder result);

        #endregion

        public string Name
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_channel_name(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string Description
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_description(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string CustomDataJson
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_custom_data_json(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string Updated
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_updated(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string Status
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_status(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string Type
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_type(pointer, buffer);
                return buffer.ToString();
            }
        }

        private Chat chat;
        private bool connected;

        public event Action<Message> OnMessageReceived;
        public event Action<Channel> OnChannelUpdate;
        public event Action<List<string>> OnPresenceUpdate;

        internal Channel(Chat chat, string channelId, IntPtr channelPointer) : base(channelPointer, channelId)
        {
            this.chat = chat;
        }

        internal static string GetChannelIdFromPtr(IntPtr channelPointer)
        {
            var buffer = new StringBuilder(512);
            pn_channel_get_channel_id(channelPointer, buffer);
            return buffer.ToString();
        }

        internal void BroadcastMessageReceived(Message message)
        {
            if (connected)
            {
                OnMessageReceived?.Invoke(message);
            }
        }

        internal void BroadcastChannelUpdate()
        {
            //TODO: is this check necessary?
            if (connected)
            {
                OnChannelUpdate?.Invoke(this);
            }
        }

        internal void BroadcastPresenceUpdate()
        {
            if (connected)
            {
                OnPresenceUpdate?.Invoke(WhoIsPresent());
            }
        }

        public void Connect()
        {
            connected = true;
            OnMessageReceived = null;
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_connect(pointer, messagesBuffer));
            chat.ParseJsonUpdatePointers(messagesBuffer.ToString());
        }
        
        public void Join()
        {
            connected = true;
            OnMessageReceived = null;
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_join(pointer, string.Empty, messagesBuffer));
            chat.ParseJsonUpdatePointers(messagesBuffer.ToString());
        }

        public void Disconnect()
        {
            connected = false;
            CUtilities.CheckCFunctionResult(pn_channel_disconnect(pointer));
        }

        public void Leave()
        {
            connected = false;
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

        //TODO: wrap further?
        public string GetUserRestrictions(string userId, int limit, string startTimetoken, string endTimetoken)
        {
            var buffer = new StringBuilder(4096);
            CUtilities.CheckCFunctionResult(pn_channel_get_user_restrictions(pointer, userId, Id, limit, startTimetoken,
                endTimetoken, buffer));
            return buffer.ToString();
        }

        public bool IsUserPresent(string userId)
        {
            var result = pn_channel_is_present(pointer, userId);
            CUtilities.CheckCFunctionResult(result);
            return result == 1;
        }

        public List<string> WhoIsPresent()
        {
            var buffer = new StringBuilder(4096);
            CUtilities.CheckCFunctionResult(pn_channel_who_is_present(pointer, buffer));
            var jsonResult = buffer.ToString();
            var ret = new List<string>();
            if (!string.IsNullOrEmpty(jsonResult) && jsonResult != "[]" && jsonResult != "{}")
            {
                ret = JsonConvert.DeserializeObject<List<string>>(jsonResult);
                ret ??= new List<string>();
            }

            return ret;
        }

        public List<Membership> GetMemberships(int limit, string startTimeToken, string endTimeToken)
        {
            return chat.GetChannelMemberships(Id, limit, startTimeToken, endTimeToken);
        }

        public bool TryGetMessage(string timeToken, out Message message)
        {
            return chat.TryGetMessage(Id, timeToken, out message);
        }

        protected override void DisposePointer()
        {
            pn_channel_delete(pointer);
        }
    }
}