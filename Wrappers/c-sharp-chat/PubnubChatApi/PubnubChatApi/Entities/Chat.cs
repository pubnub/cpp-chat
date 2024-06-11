using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
        private static extern int pn_chat_get_messages(IntPtr chat, string channel_id, StringBuilder messages_json);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_c_get_error_message(StringBuilder buffer);
        
        #endregion
        
        private IntPtr chatPointer;
        private Dictionary<string, Channel> channels = new();
        private Dictionary<string, User> users = new();

        public Chat(string publishKey, string subscribeKey, string userId)
        {
            chatPointer = pn_chat_new(publishKey, subscribeKey, userId);
        }

        public Channel CreatePublicConversation(string channelId)
        {
            return CreatePublicConversation(channelId, new ChatChannelData());
        }

        //TODO: return actual Message objects
        public string GetMessages(string channelId)
        {
            var messagesBuffer = new StringBuilder(32768);
            if (pn_chat_get_messages(chatPointer, channelId, messagesBuffer) == -1)
            {
                ThrowCError();
            }
            return messagesBuffer.ToString();;
        }

        public Channel CreatePublicConversation(string channelId, ChatChannelData additionalData)
        {
            if (channels.TryGetValue(channelId, out var existingChannel))
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
            if(channelPointer == IntPtr.Zero)
            {
                ThrowCError();
            }
            var channel = new Channel(this, channelId, channelPointer);
            channels.Add(channelId, channel);
            return channel;
        }

        public bool TryGetChannel(string channelId, out Channel channel)
        {
            var foundChannel = channels.TryGetValue(channelId, out channel);
            if (foundChannel && channel == null)
            {
                channels.Remove(channelId);
                return false;
            }

            return foundChannel;
        }

        public void UpdateChannel(string channelId, ChatChannelData updatedData)
        {
            pn_chat_update_channel_dirty(chatPointer, channelId, updatedData.ChannelName,
                updatedData.ChannelDescription,
                updatedData.ChannelCustomDataJson,
                updatedData.ChannelUpdated,
                updatedData.ChannelStatus,
                updatedData.ChannelType);
        }

        public void DeleteChannel(string channelId)
        {
            if (channels.ContainsKey(channelId))
            {
                channels.Remove(channelId);
                if (pn_chat_delete_channel(chatPointer, channelId) == -1)
                {
                    ThrowCError();
                }
            }
        }

        public void SetRestrictions(string userId, string channelId, bool banUser, bool muteUser, string reason)
        {
            if (pn_chat_set_restrictions(chatPointer, userId, channelId, banUser, muteUser, reason) == -1)
            {
                ThrowCError();
            }
        }

        public User CreateUser(string userId)
        {
            return CreateUser(userId, new ChatUserData());
        }

        public User CreateUser(string userId, ChatUserData additionalData)
        {
            if (users.TryGetValue(userId, out var existingUser))
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
            var user = new User(this, userId, userPointer);
            users.Add(userId, user);
            return user;
        }

        public bool TryGetUser(string userId, out User user)
        {
            var foundUser = users.TryGetValue(userId, out user);
            if (foundUser && user == null)
            {
                users.Remove(userId);
                return false;
            }

            return foundUser;
        }

        public void UpdateUser(string userId, ChatUserData updatedData)
        {
            pn_chat_update_user_dirty(chatPointer, userId, 
                updatedData.Username,
                updatedData.ExternalId, 
                updatedData.ProfileUrl, 
                updatedData.Email,
                updatedData.CustomDataJson, 
                updatedData.Status, 
                updatedData.Status);
        }

        public void DeleteUser(string userId)
        {
            if (users.ContainsKey(userId))
            {
                users.Remove(userId);
                if (pn_chat_delete_user(chatPointer, userId) == -1)
                {
                    ThrowCError();
                }
            }
        }
        
        private void ThrowCError()
        {
            var buffer = new StringBuilder(4096);
            pn_c_get_error_message(buffer);
            throw new Exception(buffer.ToString());
        }

        ~Chat()
        {
            pn_chat_delete(chatPointer);
        }
    }
}