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
    /// <summary>
    /// Data class for the chat user.
    /// <para>
    /// Contains all the data related to the chat user.
    /// </para>
    /// </summary>
    /// <remarks>
    /// By default, all the properties are set to empty strings.
    /// </remarks>
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
    /// <summary>
    /// Data class for the chat channel.
    /// <para>
    /// Contains all the data related to the chat channel.
    /// </para>
    /// </summary>
    /// <remarks>
    /// By default, all the properties are set to empty strings.
    /// </remarks>
    public class ChatChannelData
    {
        public string ChannelName { get; set; } = string.Empty;
        public string ChannelDescription { get; set; } = string.Empty;
        public string ChannelCustomDataJson { get; set; } = string.Empty;
        public string ChannelUpdated { get; set; } = string.Empty;
        public string ChannelStatus { get; set; } = string.Empty;
        public string ChannelType { get; set; } = string.Empty;
    }

    /// <summary>
    /// Main class for the chat.
    /// <para>
    /// Contains all the methods to interact with the chat.
    /// It should be treated as a root of the chat system.
    /// </para>
    /// </summary>
    /// <remarks>
    /// The class is responsible for creating and managing channels, users, and messages.
    /// </remarks>
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

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_chat_get_users(
            IntPtr chat,
            string include,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_get_user(
            IntPtr chat,
            string user_id);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_chat_get_channel(
            IntPtr chat,
            string channel_id);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_user_get_memberhips(
            IntPtr user,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_get_members(
            IntPtr channel,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern IntPtr pn_channel_get_message(IntPtr channel, string timetoken);

        [DllImport("pubnub-chat.dll")]
        private static extern int pn_channel_get_history(
            IntPtr channel,
            string start,
            string end,
            int count,
            StringBuilder result);

        #endregion

        private IntPtr chatPointer;
        private Dictionary<string, Channel> channelWrappers = new();
        private Dictionary<string, User> userWrappers = new();
        private Dictionary<string, Membership> membershipWrappers = new();
        private Dictionary<string, Message> messageWrappers = new();
        private bool fetchUpdates = true;
        private Thread fetchUpdatesThread;

        public event Action<string> OnEvent;

        /// <summary>
        /// Initializes a new instance of the <see cref="Chat"/> class.
        /// <para>
        /// Creates a new chat instance.
        /// </para>
        /// </summary>
        /// <param name="publishKey">The PubNub publish key.</param>
        /// <param name="subscribeKey">The PubNub subscribe key.</param>
        /// <param name="userId">The user ID.</param>
        /// <remarks>
        /// The constructor initializes the chat instance with the provided keys and user ID.
        /// </remarks>
        public Chat(string publishKey, string subscribeKey, string userId)
        {
            chatPointer = pn_chat_new(publishKey, subscribeKey, userId);
            CUtilities.CheckCFunctionResult(chatPointer);

            fetchUpdatesThread = new Thread(FetchUpdatesLoop) { IsBackground = true };
            fetchUpdatesThread.Start();
        }

        #region Updates handling

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
                        //TODO: later Event will be a proper PointerWrapper class
                        OnEvent?.Invoke(stringUpdateBuffer.ToString());
                        pn_dispose_message(pointer);
                        continue;
                    }

                    //Presence (json list of uuids)
                    if (pn_deserialize_presence(pointer, stringUpdateBuffer) != -1)
                    {
                        var channelId = stringUpdateBuffer.ToString();
                        if (TryGetChannel(channelId, out var channel))
                        {
                            channel.BroadcastPresenceUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }
                }
            }
        }

        internal string GetUpdates()
        {
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_chat_get_updates(chatPointer, messagesBuffer));
            return messagesBuffer.ToString();
        }

        #endregion

        #region Channels

        /// <summary>
        /// Creates a new public conversation.
        /// <para>
        /// Creates a new public conversation with the provided channel ID.
        /// Conversation allows users to interact with each other.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <returns>The created channel.</returns>
        /// <remarks>
        /// The method creates a chat channel with the provided channel ID.
        /// </remarks>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var channel = chat.CreatePublicConversation("channel_id");
        /// </code>
        /// </example>
        /// <seealso cref="Channel"/>
        public Channel CreatePublicConversation(string channelId)
        {
            return CreatePublicConversation(channelId, new ChatChannelData());
        }

        /// <summary>
        /// Creates a new public conversation.
        /// <para>
        /// Creates a new public conversation with the provided channel ID.
        /// Conversation allows users to interact with each other.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="additionalData">The additional data for the channel.</param>
        /// <returns>The created channel.</returns>
        /// <remarks>
        /// The method creates a chat channel with the provided channel ID.
        /// </remarks>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var channel = chat.CreatePublicConversation("channel_id");
        /// </code>
        /// </example>
        /// <seealso cref="Channel"/>
        /// <seealso cref="ChatChannelData"/>
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

        /// <summary>
        /// Gets the channel by the provided channel ID.
        /// <para>
        /// Tries to get the channel by the provided channel ID.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="channel">The out channel.</param>
        /// <returns>True if the channel was found, false otherwise.</returns>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// if (chat.TryGetChannel("channel_id", out var channel)) {
        ///    // Channel found
        /// }
        /// </code>
        /// </example>
        /// <seealso cref="Channel"/>
        public bool TryGetChannel(string channelId, out Channel channel)
        {
            var channelPointer = pn_chat_get_channel(chatPointer, channelId);
            return TryGetChannel(channelId, channelPointer, out channel);
        }

        private bool TryGetChannel(string channelId, IntPtr channelPointer, out Channel channel)
        {
            if (channelWrappers.TryGetValue(channelId, out channel))
            {
                //We had it before but it's no longer valid
                if (channelPointer == IntPtr.Zero)
                {
                    Debug.WriteLine(CUtilities.GetErrorMessage());
                    channelWrappers.Remove(channelId);
                    return false;
                }

                //Pointer is valid but something nulled the wrapper
                if (channel == null)
                {
                    channelWrappers[channelId] = new Channel(this, channelId, channelPointer);
                    channel = channelWrappers[channelId];
                    return true;
                }
                //Updating existing wrapper with updated pointer
                else
                {
                    channel.UpdatePointer(channelPointer);
                    return true;
                }
            }
            //Adding new user to wrappers cache
            else if (channelPointer != IntPtr.Zero)
            {
                channel = new Channel(this, channelId, channelPointer);
                channelWrappers.Add(channelId, channel);
                return true;
            }
            else
            {
                Debug.WriteLine(CUtilities.GetErrorMessage());
                return false;
            }
        }

        /// <summary>
        /// Updates the channel with the provided channel ID.
        /// <para>
        /// Updates the channel with the provided channel ID with the provided data.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="updatedData">The updated data for the channel.</param>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the channel with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// chat.UpdateChannel("channel_id", new ChatChannelData {
        ///    ChannelName = "new_name"
        ///    // ...
        ///  });
        /// </code>
        /// </example>
        /// <seealso cref="ChatChannelData"/>
        public void UpdateChannel(string channelId, ChatChannelData updatedData)
        {
            var newPointer = pn_chat_update_channel_dirty(chatPointer, channelId, updatedData.ChannelName,
                updatedData.ChannelDescription,
                updatedData.ChannelCustomDataJson,
                updatedData.ChannelUpdated,
                updatedData.ChannelStatus,
                updatedData.ChannelType);
            CUtilities.CheckCFunctionResult(newPointer);
            if (channelWrappers.TryGetValue(channelId, out var existingChannelWrapper))
            {
                existingChannelWrapper.UpdatePointer(newPointer);
            }
            else
            {
                channelWrappers.Add(channelId, new Channel(this, channelId, newPointer));
            }
        }

        /// <summary>
        /// Deletes the channel with the provided channel ID.
        /// <para>
        /// The channel is deleted with all the messages and users.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the channel with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// chat.DeleteChannel("channel_id");
        /// </code>
        /// </example>
        public void DeleteChannel(string channelId)
        {
            if (channelWrappers.ContainsKey(channelId))
            {
                channelWrappers.Remove(channelId);
                CUtilities.CheckCFunctionResult(pn_chat_delete_channel(chatPointer, channelId));
            }
        }

        #endregion

        #region Users

        /// <summary>
        /// Sets the restrictions for the user with the provided user ID.
        /// <para>
        /// Sets the restrictions for the user with the provided user ID in the provided channel.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="banUser">The ban user flag.</param>
        /// <param name="muteUser">The mute user flag.</param>
        /// <param name="reason">The reason for the restrictions.</param>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the user with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// chat.SetRestrictions("user_id", "channel_id", true, true, "Spamming");
        /// </code>
        /// </example>
        public void SetRestrictions(string userId, string channelId, bool banUser, bool muteUser, string reason)
        {
            CUtilities.CheckCFunctionResult(
                pn_chat_set_restrictions(chatPointer, userId, channelId, banUser, muteUser, reason));
        }

        /// <summary>
        /// Creates a new user with the provided user ID.
        /// <para>
        /// Creates a new user with the empty data and the provided user ID.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <returns>The created user.</returns>
        /// <remarks>
        /// The data for user is empty.
        /// </remarks>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var user = chat.CreateUser("user_id");
        /// </code>
        /// </example>
        /// <seealso cref="User"/>
        public User CreateUser(string userId)
        {
            return CreateUser(userId, new ChatUserData());
        }

        /// <summary>
        /// Creates a new user with the provided user ID.
        /// <para>
        /// Creates a new user with the provided data and the provided user ID.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <param name="additionalData">The additional data for the user.</param>
        /// <returns>The created user.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var user = chat.CreateUser("user_id");
        /// </code>
        /// </example>
        /// <seealso cref="User"/>
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

        public bool IsPresent(string userId, string channelId)
        {
            if (TryGetChannel(channelId, out var channel))
            {
                return channel.IsUserPresent(userId);
            }
            else
            {
                return false;
            }
        }

        public List<string> WhoIsPresent(string channelId)
        {
            if (TryGetChannel(channelId, out var channel))
            {
                return channel.WhoIsPresent();
            }
            else
            {
                return new List<string>();
            }
        }

        public List<string> WherePresent(string userId)
        {
            if (TryGetUser(userId, out var user))
            {
                return user.WherePresent();
            }
            else
            {
                return new List<string>();
            }
        }

        public bool TryGetUser(string userId, out User user)
        {
            var userPointer = pn_chat_get_user(chatPointer, userId);
            return TryGetUser(userId, userPointer, out user);
        }

        private bool TryGetUser(string userId, IntPtr userPointer, out User user)
        {
            if (userWrappers.TryGetValue(userId, out user))
            {
                //We had it before but it's no longer valid
                if (userPointer == IntPtr.Zero)
                {
                    Debug.WriteLine(CUtilities.GetErrorMessage());
                    userWrappers.Remove(userId);
                    return false;
                }

                //Pointer is valid but something nulled the wrapper
                if (user == null)
                {
                    userWrappers[userId] = new User(this, userId, userPointer);
                    user = userWrappers[userId];
                    return true;
                }
                //Updating existing wrapper with updated pointer
                else
                {
                    user.UpdatePointer(userPointer);
                    return true;
                }
            }
            //Adding new user to wrappers cache
            else if (userPointer != IntPtr.Zero)
            {
                user = new User(this, userId, userPointer);
                userWrappers.Add(userId, user);
                return true;
            }
            else
            {
                Debug.WriteLine(CUtilities.GetErrorMessage());
                return false;
            }
        }

        public List<User> GetUsers(string include, int limit, string startTimeToken, string endTimeToken)
        {
            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_chat_get_users(chatPointer, include, limit, startTimeToken, endTimeToken,
                buffer));
            var jsonPointers = buffer.ToString();
            var userPointers = JsonConvert.DeserializeObject<IntPtr[]>(jsonPointers);
            var returnUsers = new List<User>();
            if (userPointers == null)
            {
                return returnUsers;
            }

            foreach (var userPointer in userPointers)
            {
                var id = User.GetUserIdFromPtr(userPointer);
                if (TryGetUser(id, userPointer, out var user))
                {
                    returnUsers.Add(user);
                }
            }

            return returnUsers;
        }

        public void UpdateUser(string userId, ChatUserData updatedData)
        {
            var newPointer = pn_chat_update_user_dirty(chatPointer, userId,
                updatedData.Username,
                updatedData.ExternalId,
                updatedData.ProfileUrl,
                updatedData.Email,
                updatedData.CustomDataJson,
                updatedData.Status,
                updatedData.Status);
            CUtilities.CheckCFunctionResult(newPointer);
            if (userWrappers.TryGetValue(userId, out var existingUserWrapper))
            {
                existingUserWrapper.UpdatePointer(newPointer);
            }
            else
            {
                userWrappers.Add(userId, new User(this, userId, newPointer));
            }
        }

        public void DeleteUser(string userId)
        {
            if (userWrappers.ContainsKey(userId))
            {
                userWrappers.Remove(userId);
                CUtilities.CheckCFunctionResult(pn_chat_delete_user(chatPointer, userId));
            }
        }

        #endregion

        #region Memberships

        public List<Membership> GetUserMemberships(string userId, int limit, string startTimeToken, string endTimeToken)
        {
            if (!TryGetUser(userId, out var user))
            {
                return new List<Membership>();
            }

            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_user_get_memberhips(user.Pointer, limit, startTimeToken, endTimeToken,
                buffer));
            return ParseJsonMembershipPointers(buffer.ToString());
        }

        public List<Membership> GetChannelMemberships(string channelId, int limit, string startTimeToken,
            string endTimeToken)
        {
            if (!TryGetChannel(channelId, out var channel))
            {
                return new List<Membership>();
            }

            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_channel_get_members(channel.Pointer, limit, startTimeToken, endTimeToken,
                buffer));
            return ParseJsonMembershipPointers(buffer.ToString());
        }

        private List<Membership> ParseJsonMembershipPointers(string membershipPointersJson)
        {
            var memberships = new List<Membership>();
            if (!string.IsNullOrEmpty(membershipPointersJson) && membershipPointersJson != "[]" &&
                membershipPointersJson != "{}")
            {
                var membershipPointers = JsonConvert.DeserializeObject<IntPtr[]>(membershipPointersJson);
                if (membershipPointers == null)
                {
                    return memberships;
                }

                foreach (var membershipPointer in membershipPointers)
                {
                    var id = Membership.GetMembershipIdFromPtr(membershipPointer);
                    if (TryGetMembership(id, membershipPointer, out var membership))
                    {
                        memberships.Add(membership);
                    }
                }
            }

            return memberships;
        }

        private bool TryGetMembership(string membershipId, IntPtr membershipPointer, out Membership membership)
        {
            if (membershipWrappers.TryGetValue(membershipId, out membership))
            {
                //We had it before but it's no longer valid
                if (membershipPointer == IntPtr.Zero)
                {
                    Debug.WriteLine(CUtilities.GetErrorMessage());
                    membershipWrappers.Remove(membershipId);
                    return false;
                }

                //Pointer is valid but something nulled the wrapper
                if (membership == null)
                {
                    membershipWrappers[membershipId] = new Membership(membershipPointer, membershipId);
                    membership = membershipWrappers[membershipId];
                    return true;
                }
                //Updating existing wrapper with updated pointer
                else
                {
                    membership.UpdatePointer(membershipPointer);
                    return true;
                }
            }
            //Adding new user to wrappers cache
            else if (membershipPointer != IntPtr.Zero)
            {
                membership = new Membership(membershipPointer, membershipId);
                membershipWrappers.Add(membershipId, membership);
                return true;
            }
            else
            {
                Debug.WriteLine(CUtilities.GetErrorMessage());
                return false;
            }
        }

        #endregion

        #region Messages

        public bool TryGetMessage(string channelId, string messageTimeToken, out Message message)
        {
            if (!TryGetChannel(channelId, out var channel))
            {
                message = null;
                return false;
            }

            var messagePointer = pn_channel_get_message(channel.Pointer, messageTimeToken);
            return TryGetMessage(messageTimeToken, messagePointer, out message);
        }

        private bool TryGetMessage(string timeToken, IntPtr messagePointer, out Message message)
        {
            if (messageWrappers.TryGetValue(timeToken, out message))
            {
                //We had it before but it's no longer valid
                if (messagePointer == IntPtr.Zero)
                {
                    Debug.WriteLine(CUtilities.GetErrorMessage());
                    messageWrappers.Remove(timeToken);
                    return false;
                }

                //Pointer is valid but something nulled the wrapper
                if (message == null)
                {
                    messageWrappers[timeToken] = new Message(this, messagePointer, timeToken);
                    message = messageWrappers[timeToken];
                    return true;
                }
                //Updating existing wrapper with updated pointer
                else
                {
                    message.UpdatePointer(messagePointer);
                    return true;
                }
            }
            //Adding new user to wrappers cache
            else if (messagePointer != IntPtr.Zero)
            {
                message = new Message(this, messagePointer, timeToken);
                messageWrappers.Add(timeToken, message);
                return true;
            }
            else
            {
                Debug.WriteLine(CUtilities.GetErrorMessage());
                return false;
            }
        }

        public List<Message> GetChannelMessageHistory(string channelId, string startTimeToken, string endTimeToken,
            int count)
        {
            var messages = new List<Message>();
            if (!TryGetChannel(channelId, out var channel))
            {
                Debug.WriteLine("Didn't find the channel for history fetch!");
                return messages;
            }

            var buffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_get_history(channel.Pointer, startTimeToken, endTimeToken, count,
                buffer));
            var jsonPointers = buffer.ToString();
            var messagePointers = JsonConvert.DeserializeObject<IntPtr[]>(jsonPointers);
            var returnMessages = new List<Message>();
            if (messagePointers == null)
            {
                return returnMessages;
            }

            foreach (var messagePointer in messagePointers)
            {
                var id = Message.GetMessageIdFromPtr(messagePointer);
                if (TryGetMessage(id, messagePointer, out var message))
                {
                    returnMessages.Add(message);
                }
            }

            return returnMessages;
        }

        #endregion

        ~Chat()
        {
            fetchUpdates = false;
            fetchUpdatesThread.Join();
            pn_chat_delete(chatPointer);
        }
    }
}
