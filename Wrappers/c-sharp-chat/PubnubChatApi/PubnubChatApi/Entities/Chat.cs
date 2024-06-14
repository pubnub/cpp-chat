using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Newtonsoft.Json;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    //TODO: move out of this file
    public class ChatUserData
    {
        public string Username { get; set; } = string.Empty;
        public string ExternalId { get; set; } = string.Empty;
        public string ProfileUrl { get; set; } = string.Empty;
        public string Email { get; set; } = string.Empty;
        public string CustomDataJson { get; set; } = string.Empty;
        public string Status { get; set; } = string.Empty;
        public string Type { get; set; } = string.Empty;
    }

    //TODO: move out of this file
    public class ChatChannelData
    {
        public string ChannelName { get; set; } = string.Empty;
        public string ChannelDescription { get; set; } = string.Empty;
        public string ChannelCustomDataJson { get; set; } = string.Empty;
        public string ChannelUpdated { get; set; } = string.Empty;
        public string ChannelStatus { get; set; } = string.Empty;
        public string ChannelType { get; set; } = string.Empty;
    }

    public class Chat
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_new(string publish, string subscribe, string user_id);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_chat_delete(IntPtr chat);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_create_public_conversation_dirty(IntPtr chat,
            string channel_id,
            string channel_name,
            string channel_description,
            string channel_custom_data_json,
            string channel_updated,
            string channel_status,
            string channel_type);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_update_channel_dirty(IntPtr chat,
            string channel_id,
            string channel_name,
            string channel_description,
            string channel_custom_data_json,
            string channel_updated,
            string channel_status,
            string channel_type);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_chat_set_restrictions(IntPtr chat,
            string user_id,
            string channel_id,
            bool ban_user,
            bool mute_user,
            string reason);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_chat_delete_channel(IntPtr chat, string channel_id);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_create_user_dirty(IntPtr chat,
            string user_id,
            string user_name,
            string external_id,
            string profile_url,
            string email,
            string custom_data_json,
            string status,
            string type);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_update_user_dirty(IntPtr chat,
            string user_id,
            string user_name,
            string external_id,
            string profile_url,
            string email,
            string custom_data_json,
            string status,
            string type);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_chat_delete_user(IntPtr chat, string user_id);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_chat_get_updates(IntPtr chat, StringBuilder messages_json);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_deserialize_message(IntPtr chat, IntPtr message);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_deserialize_channel(IntPtr chat, IntPtr channel);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_deserialize_user(IntPtr chat, IntPtr user);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_deserialize_membership(IntPtr chat, IntPtr membership);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_deserialize_message_update(IntPtr chat, IntPtr message);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_deserialize_event(IntPtr eventPtr, StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_deserialize_presence(IntPtr presence, StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_dispose_message(IntPtr message);

        #endregion

        private IntPtr chatPointer;
        private Dictionary<string, Channel> channelWrappers = new();
        private Dictionary<string, User> userWrappers = new();
        private Dictionary<string, Message> messageWrappers = new();
        private Dictionary<string, Membership> membershipWrappers = new();
        private bool fetchUpdates = true;
        private Thread fetchUpdatesThread;
        
        //TODO: wrap these further?
        public event Action<List<string>> OnPresenceUpdate; 
        public event Action<string> OnEvent; 

        public Chat(string publishKey, string subscribeKey, string userId)
        {
            chatPointer = pn_chat_new(publishKey, subscribeKey, userId);
            CUtilities.CheckCFunctionResult(chatPointer);

            fetchUpdatesThread = new Thread(FetchUpdatesLoop) { IsBackground = true };
            fetchUpdatesThread.Start();
        }

        private void FetchUpdatesLoop()
        {
            while (fetchUpdates)
            {
                var updates = GetUpdates();
                ParseJsonUpdatePointers(updates);
                Thread.Sleep(500);
            }
        }

        internal void ParseJsonUpdatePointers(string jsonPointers)
        {
            if (!string.IsNullOrEmpty(jsonPointers) && jsonPointers != "[]")
            {
                var pubnubV2MessagePointers = JsonConvert.DeserializeObject<IntPtr[]>(jsonPointers);
                if (pubnubV2MessagePointers == null)
                {
                    return;
                }

                foreach (var pointer in pubnubV2MessagePointers)
                {
                    //New message
                    var messagePointer = pn_deserialize_message(chatPointer, pointer);
                    if (messagePointer != IntPtr.Zero)
                    {
                        var id = Message.GetChannelIdFromMessagePtr(messagePointer);
                        if (channelWrappers.TryGetValue(id, out var channel))
                        {
                            var timeToken = Message.GetMessageIdFromPtr(messagePointer);
                            var message = new Message(this, messagePointer, timeToken);
                            messageWrappers[timeToken] = message;
                            channel.BroadcastMessageReceived(message);
                        }
                        pn_dispose_message(pointer);
                        continue;
                    }

                    //Updated existing message
                    var updatedMessagePointer = pn_deserialize_message_update(chatPointer, pointer);
                    if (updatedMessagePointer != IntPtr.Zero)
                    {
                        var id = Message.GetMessageIdFromPtr(updatedMessagePointer);
                        if (messageWrappers.TryGetValue(id, out var existingMessageWrapper))
                        {
                            existingMessageWrapper.UpdatePointer(updatedMessagePointer);
                            existingMessageWrapper.BroadcastMessageUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

                    //Updated channel
                    var channelPointer = pn_deserialize_channel(chatPointer, pointer);
                    if (channelPointer != IntPtr.Zero)
                    {
                        var id = Channel.GetChannelIdFromPtr(channelPointer);
                        if (channelWrappers.TryGetValue(id, out var existingChannelWrapper))
                        {
                            existingChannelWrapper.UpdatePointer(channelPointer);
                            existingChannelWrapper.BroadcastChannelUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

                    //Updated user
                    var userPointer = pn_deserialize_user(chatPointer, pointer);
                    if (userPointer != IntPtr.Zero)
                    {
                        var id = User.GetUserIdFromPtr(userPointer);
                        if (userWrappers.TryGetValue(id, out var existingUserWrapper))
                        {
                            existingUserWrapper.UpdatePointer(userPointer);
                            existingUserWrapper.BroadcastUserUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

                    //Updated membership
                    var membershipPointer = pn_deserialize_membership(chatPointer, pointer);
                    if (membershipPointer != IntPtr.Zero)
                    {
                        var id = Membership.GetMembershipIdFromPtr(membershipPointer);
                        if (membershipWrappers.TryGetValue(id, out var existingMembershipWrapper))
                        {
                            existingMembershipWrapper.UpdatePointer(membershipPointer);
                            existingMembershipWrapper.BroadcastMembershipUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

                    var stringUpdateBuffer = new StringBuilder(16384);

                    //Event (json)
                    if (pn_deserialize_event(pointer, stringUpdateBuffer) != -1)
                    {
                        OnEvent?.Invoke(stringUpdateBuffer.ToString());
                        pn_dispose_message(pointer);
                        continue;
                    }

                    //Presence (json list of uuids)
                    if (pn_deserialize_presence(pointer, stringUpdateBuffer) != -1)
                    {
                        var uuidsJson = stringUpdateBuffer.ToString();
                        if (!string.IsNullOrEmpty(uuidsJson) && uuidsJson != "[]" && uuidsJson != "{}")
                        {
                            var uuids = JsonConvert.DeserializeObject<List<string>>(stringUpdateBuffer.ToString());
                            if (uuids != null)
                            {
                                OnPresenceUpdate?.Invoke(uuids);   
                            }
                        }
                        pn_dispose_message(pointer);
                        continue;
                    }
                }
            }
        }

        public Channel CreatePublicConversation(string channelId)
        {
            return CreatePublicConversation(channelId, new ChatChannelData());
        }
        
        internal string GetUpdates()
        {
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_chat_get_updates(chatPointer, messagesBuffer));
            return messagesBuffer.ToString();
        }

        public Channel CreatePublicConversation(string channelId, ChatChannelData additionalData)
        {
            if (channelWrappers.TryGetValue(channelId, out var existingChannel))
            {
                Debug.WriteLine("Trying to create a channel with ID that already exists! Returning existing one.");
                return existingChannel;
            }

            var channelPointer = pn_chat_create_public_conversation_dirty(chatPointer, channelId,
                additionalData.ChannelName,
                additionalData.ChannelDescription,
                additionalData.ChannelCustomDataJson,
                additionalData.ChannelUpdated,
                additionalData.ChannelStatus,
                additionalData.ChannelType);
            CUtilities.CheckCFunctionResult(channelPointer);
            var channel = new Channel(this, channelId, channelPointer);
            channelWrappers.Add(channelId, channel);
            return channel;
        }

        public bool TryGetChannel(string channelId, out Channel channel)
        {
            var foundChannel = channelWrappers.TryGetValue(channelId, out channel);
            if (foundChannel && channel == null)
            {
                channelWrappers.Remove(channelId);
                return false;
            }

            return foundChannel;
        }

        public void UpdateChannel(string channelId, ChatChannelData updatedData)
        {
            CUtilities.CheckCFunctionResult(
                pn_chat_update_channel_dirty(chatPointer, channelId, updatedData.ChannelName,
                    updatedData.ChannelDescription,
                    updatedData.ChannelCustomDataJson,
                    updatedData.ChannelUpdated,
                    updatedData.ChannelStatus,
                    updatedData.ChannelType));
        }

        public void DeleteChannel(string channelId)
        {
            if (channelWrappers.ContainsKey(channelId))
            {
                channelWrappers.Remove(channelId);
                CUtilities.CheckCFunctionResult(pn_chat_delete_channel(chatPointer, channelId));
            }
        }

        public void SetRestrictions(string userId, string channelId, bool banUser, bool muteUser, string reason)
        {
            CUtilities.CheckCFunctionResult(
                pn_chat_set_restrictions(chatPointer, userId, channelId, banUser, muteUser, reason));
        }

        public User CreateUser(string userId)
        {
            return CreateUser(userId, new ChatUserData());
        }

        public User CreateUser(string userId, ChatUserData additionalData)
        {
            if (userWrappers.TryGetValue(userId, out var existingUser))
            {
                Debug.WriteLine("Trying to create a user with ID that already exists! Returning existing one.");
                return existingUser;
            }

            var userPointer = pn_chat_create_user_dirty(chatPointer, userId,
                additionalData.Username,
                additionalData.ExternalId,
                additionalData.ProfileUrl,
                additionalData.Email,
                additionalData.CustomDataJson,
                additionalData.Status,
                additionalData.Status);
            CUtilities.CheckCFunctionResult(userPointer);
            var user = new User(this, userId, userPointer);
            userWrappers.Add(userId, user);
            return user;
        }

        public bool TryGetUser(string userId, out User user)
        {
            var foundUser = userWrappers.TryGetValue(userId, out user);
            if (foundUser && user == null)
            {
                userWrappers.Remove(userId);
                return false;
            }

            return foundUser;
        }

        public void UpdateUser(string userId, ChatUserData updatedData)
        {
            CUtilities.CheckCFunctionResult(
                pn_chat_update_user_dirty(chatPointer, userId,
                    updatedData.Username,
                    updatedData.ExternalId,
                    updatedData.ProfileUrl,
                    updatedData.Email,
                    updatedData.CustomDataJson,
                    updatedData.Status,
                    updatedData.Status));
        }

        public void DeleteUser(string userId)
        {
            if (userWrappers.ContainsKey(userId))
            {
                userWrappers.Remove(userId);
                CUtilities.CheckCFunctionResult(pn_chat_delete_user(chatPointer, userId));
            }
        }

        ~Chat()
        {
            fetchUpdates = false;
            fetchUpdatesThread.Join();
            pn_chat_delete(chatPointer);
        }
    }
}