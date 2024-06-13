using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Newtonsoft.Json;
using PubnubChatApi.Enums;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class Channel
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

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_deserialize_message(IntPtr chat, IntPtr message);
        
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_dispose_message(IntPtr message);

        #endregion

        private IntPtr channelPointer;
        private Chat chat;
        private bool fetchMessages = true;
        private Thread messageFetchingThread;

        public string ChannelId { get; }
        public event Action<Message> OnMessageReceived;
        /*public event Action<Event> OnEventReceived;
        public event Action<Event> OnEventReceived;*/

        internal Channel(Chat chat, string channelId, IntPtr channelPointer)
        {
            this.chat = chat;
            ChannelId = channelId;
            this.channelPointer = channelPointer;

            messageFetchingThread = new Thread(FetchMessagesLoop);
            messageFetchingThread.Start();
        }

        internal void UpdatePointer(IntPtr newChannelPointer)
        {
           pn_channel_delete(channelPointer);
           channelPointer = newChannelPointer;
        }

        //TODO: MOVE TO CHAT, CHANNEL ID DOES NOTHING - REMOVE IT FROM SIGNATURE
        private void FetchMessagesLoop()
        {
            while (fetchMessages)
            {
                var messages = chat.GetMessages(ChannelId);
                if (!string.IsNullOrEmpty(messages) && messages != "[]")
                {
                    var pubnubV2MessagePointers = JsonConvert.DeserializeObject<IntPtr[]>(messages);
                    if (pubnubV2MessagePointers == null)
                    {
                        continue;
                    }
                    
                    foreach (var pointer in pubnubV2MessagePointers)
                    {
                        var messagePointer = pn_deserialize_message(chat.ChatPointer, pointer);
                        if (messagePointer != IntPtr.Zero)
                        {
                            OnMessageReceived?.Invoke(new Message(chat, messagePointer));
                        }
                        pn_dispose_message(pointer);
                    }
                }
                Thread.Sleep(500);
            }
        }

        //TODO: return actual Message objects
        public string Connect()
        {
            OnMessageReceived = null;
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_connect(channelPointer, messagesBuffer));
            return messagesBuffer.ToString();
        }

        public void Join()
        {
            OnMessageReceived = null;
            CUtilities.CheckCFunctionResult(pn_channel_join(channelPointer, string.Empty));
        }

        public void Disconnect()
        {
            CUtilities.CheckCFunctionResult(pn_channel_disconnect(channelPointer));
        }

        public void Leave()
        {
            CUtilities.CheckCFunctionResult(pn_channel_leave(channelPointer));
        }

        public void SetRestrictions(string userId, bool banUser, bool muteUser, string reason)
        {
            CUtilities.CheckCFunctionResult(pn_channel_set_restrictions(channelPointer, userId, banUser, muteUser,
                reason));
        }

        public void SendText(string message)
        {
            CUtilities.CheckCFunctionResult(pn_channel_send_text(channelPointer, message,
                (byte)pubnub_chat_message_type.PCMT_TEXT, string.Empty));
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
            fetchMessages = false;
            messageFetchingThread.Join();
            pn_channel_delete(channelPointer);
        }
    }
}