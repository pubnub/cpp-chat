using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Newtonsoft.Json;
using PubnubChatApi.Entities.Data;
using PubnubChatApi.Entities.Events;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
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

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_new(string publish, string subscribe, string user_id);

        [DllImport("pubnub-chat")]
        private static extern void pn_chat_delete(IntPtr chat);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_create_public_conversation_dirty(IntPtr chat,
            string channel_id,
            string channel_name,
            string channel_description,
            string channel_custom_data_json,
            string channel_updated,
            string channel_status,
            string channel_type);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_update_channel_dirty(IntPtr chat,
            string channel_id,
            string channel_name,
            string channel_description,
            string channel_custom_data_json,
            string channel_updated,
            string channel_status,
            string channel_type);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_set_restrictions(IntPtr chat,
            string user_id,
            string channel_id,
            bool ban_user,
            bool mute_user,
            string reason);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_delete_channel(IntPtr chat, string channel_id);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_create_user_dirty(IntPtr chat,
            string user_id,
            string user_name,
            string external_id,
            string profile_url,
            string email,
            string custom_data_json,
            string status,
            string type);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_update_user_dirty(IntPtr chat,
            string user_id,
            string user_name,
            string external_id,
            string profile_url,
            string email,
            string custom_data_json,
            string status,
            string type);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_delete_user(IntPtr chat, string user_id);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_get_updates(IntPtr chat, StringBuilder messages_json);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_deserialize_message(IntPtr chat, IntPtr message);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_deserialize_channel(IntPtr chat, IntPtr channel);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_deserialize_user(IntPtr chat, IntPtr user);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_deserialize_membership(IntPtr chat, IntPtr membership);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_deserialize_message_update(IntPtr chat, IntPtr message);

        [DllImport("pubnub-chat")]
        private static extern int pn_deserialize_event(IntPtr eventPtr, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_deserialize_presence(IntPtr presence, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_dispose_message(IntPtr message);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_get_users(
            IntPtr chat,
            string include,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_get_user(
            IntPtr chat,
            string user_id);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_get_channel(
            IntPtr chat,
            string channel_id);

        [DllImport("pubnub-chat")]
        private static extern int pn_user_get_memberships(
            IntPtr user,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_get_members(
            IntPtr channel,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_channel_get_message(IntPtr channel, string timetoken);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_get_history(
            IntPtr channel,
            string start,
            string end,
            int count,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_get_channels(
            IntPtr chat,
            string include,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_listen_for_events(
            IntPtr chat,
            string channel_id);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_create_direct_conversation_dirty(
            IntPtr chat,
            IntPtr user, string channel_id,
            string channel_name,
            string channel_description,
            string channel_custom_data_json,
            string channel_updated,
            string channel_status,
            string channel_type);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_create_group_conversation_dirty(
            IntPtr chat,
            IntPtr[] users,
            int users_length, string channel_id,
            string channel_name,
            string channel_description,
            string channel_custom_data_json,
            string channel_updated,
            string channel_status,
            string channel_type);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_get_created_channel_wrapper_channel(
            IntPtr wrapper);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_chat_get_created_channel_wrapper_host_membership(
            IntPtr wrapper);

        [DllImport("pubnub-chat")]
        private static extern int pn_chat_get_created_channel_wrapper_invited_memberships(
            IntPtr wrapper, StringBuilder result_json);
        
        [DllImport("pubnub-chat")]
        private static extern void pn_chat_dispose_created_channel_wrapper(IntPtr wrapper);
        
        [DllImport("pubnub-chat")]
        private static extern int pn_chat_forward_message(IntPtr chat, IntPtr message, IntPtr channel);

        #endregion

        private IntPtr chatPointer;
        private Dictionary<string, Channel> channelWrappers = new();
        private Dictionary<string, User> userWrappers = new();
        private Dictionary<string, Membership> membershipWrappers = new();
        private Dictionary<string, Message> messageWrappers = new();
        private bool fetchUpdates = true;
        private Thread fetchUpdatesThread;
        
        public event Action<ReportEvent> OnReportEvent;
        public event Action<ModerationEvent> OnModerationEvent;
        public event Action<ChatEvent> OnAnyEvent; 

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
                    //TODO: this handling is temporary until Events are actual entities
                    //Events (json for now)
                    var allEventsBuffer = new StringBuilder(16384);
                    if (pn_deserialize_event(pointer, allEventsBuffer) != -1)
                    {
                        var eventJson = allEventsBuffer.ToString();
                        if (!CUtilities.IsValidJson(eventJson))
                        {
                            pn_dispose_message(pointer);
                            continue;
                        }

                        var eventData = JsonConvert.DeserializeObject<Dictionary<string, string>>(eventJson);
                        if (eventData != null && eventData.TryGetValue("type", out var eventType))
                        {
                            //TODO: dumb, will be replaced by getting serialized event pointer + type
                            switch (eventType)
                            {
                                case "moderation":
                                    //TODO: real pointer!
                                    var moderationEvent = new ModerationEvent(IntPtr.Zero, eventJson);
                                    OnAnyEvent?.Invoke(moderationEvent);
                                    OnModerationEvent?.Invoke(moderationEvent);
                                    break;
                                case "report":
                                    //TODO: real pointer!
                                    var reportEvent = new ReportEvent(IntPtr.Zero, eventJson);
                                    OnAnyEvent?.Invoke(reportEvent);
                                    OnReportEvent?.Invoke(reportEvent);
                                    break;
                                //TODO: should this one also be an Event Entity?
                                case "typing":
                                    if (eventData.TryGetValue("channelId", out var channelId) &&
                                        TryGetChannel(channelId, out var channel))
                                    {
                                        channel.ParseAndBroadcastTypingEvent(eventData);
                                    }
                                    break;
                            }
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

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
                            existingMembershipWrapper.BroadcastMembershipUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

                    var presenceBuffer = new StringBuilder(16384);
                    //Presence (json list of uuids)
                    if (pn_deserialize_presence(pointer, presenceBuffer) != -1)
                    {
                        var channelId = presenceBuffer.ToString();
                        if (channelId.EndsWith("-pnpres"))
                        {
                            channelId = channelId.Substring(0,
                                channelId.LastIndexOf("-pnpres", StringComparison.Ordinal));
                        }

                        if (TryGetChannel(channelId, out var channel))
                        {
                            channel.BroadcastPresenceUpdate();
                        }

                        pn_dispose_message(pointer);
                        continue;
                    }

                    pn_dispose_message(pointer);
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

        public void AddListenerToChannelsUpdate(List<string> channelIds, Action<Channel> listener)
        {
            foreach (var channelId in channelIds)
            {
                if (TryGetChannel(channelId, out var channel))
                {
                    channel.OnChannelUpdate += listener;
                }
            }
        }

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

        public (Channel createdChannel, Membership hostMembership, Membership inviteeMembership)
            CreateDirectConversation(User user, string channelId)
        {
            return CreateDirectConversation(user, channelId, new ChatChannelData());
        }

        public (Channel createdChannel, Membership hostMembership, Membership inviteeMembership)
            CreateDirectConversation(User user, string channelId, ChatChannelData channelData)
        {
            var wrapperPointer = pn_chat_create_direct_conversation_dirty(chatPointer, user.Pointer, channelId,
                channelData.ChannelName,
                channelData.ChannelDescription, channelData.ChannelCustomDataJson, channelData.ChannelUpdated,
                channelData.ChannelStatus, channelData.ChannelType);
            CUtilities.CheckCFunctionResult(wrapperPointer);

            var createdChannelPointer = pn_chat_get_created_channel_wrapper_channel(wrapperPointer);
            CUtilities.CheckCFunctionResult(createdChannelPointer);
            TryGetChannel(createdChannelPointer, out var createdChannel);
            
            var hostMembershipPointer = pn_chat_get_created_channel_wrapper_host_membership(wrapperPointer);
            CUtilities.CheckCFunctionResult(hostMembershipPointer);
            TryGetMembership(hostMembershipPointer, out var hostMembership);

            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(
                pn_chat_get_created_channel_wrapper_invited_memberships(wrapperPointer, buffer));
            var inviteeMembership = ParseJsonMembershipPointers(buffer.ToString())[0];

            pn_chat_dispose_created_channel_wrapper(wrapperPointer);
            
            return (createdChannel, hostMembership, inviteeMembership);
        }
        
        public (Channel createdChannel, Membership hostMembership, List<Membership> inviteeMemberships)
            CreateGroupConversation(List<User> users, string channelId)
        {
            return CreateGroupConversation(users, channelId, new ChatChannelData());
        }

        public (Channel createdChannel, Membership hostMembership, List<Membership> inviteeMemberships)
            CreateGroupConversation(List<User> users, string channelId, ChatChannelData channelData)
        {
            var wrapperPointer = pn_chat_create_group_conversation_dirty(chatPointer,
                users.Select(x => x.Pointer).ToArray(), users.Count, channelId, channelData.ChannelName,
                channelData.ChannelDescription, channelData.ChannelCustomDataJson, channelData.ChannelUpdated,
                channelData.ChannelStatus, channelData.ChannelType);
            CUtilities.CheckCFunctionResult(wrapperPointer);

            var createdChannelPointer = pn_chat_get_created_channel_wrapper_channel(wrapperPointer);
            CUtilities.CheckCFunctionResult(createdChannelPointer);
            TryGetChannel(createdChannelPointer, out var createdChannel);
            
            var hostMembershipPointer = pn_chat_get_created_channel_wrapper_host_membership(wrapperPointer);
            CUtilities.CheckCFunctionResult(hostMembershipPointer);
            TryGetMembership(hostMembershipPointer, out var hostMembership);

            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(
                pn_chat_get_created_channel_wrapper_invited_memberships(wrapperPointer, buffer));
            var inviteeMemberships = ParseJsonMembershipPointers(buffer.ToString());
            
            pn_chat_dispose_created_channel_wrapper(wrapperPointer);

            return (createdChannel, hostMembership, inviteeMemberships);
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

        private bool TryGetChannel(IntPtr channelPointer, out Channel channel)
        {
            var channelId = Channel.GetChannelIdFromPtr(channelPointer);
            return TryGetChannel(channelId, channelPointer, out channel);
        }

        private bool TryGetChannel(string channelId, IntPtr channelPointer, out Channel channel)
        {
            return TryGetWrapper(channelWrappers, channelId, channelPointer,
                () => new Channel(this, channelId, channelPointer), out channel);
        }

        public List<Channel> GetChannels(string include, int limit, string startTimeToken, string endTimeToken)
        {
            Debug.WriteLine("1");
            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_chat_get_channels(chatPointer, include, limit, startTimeToken,
                endTimeToken,
                buffer));
            Debug.WriteLine("2");
            var jsonPointers = buffer.ToString();
            Debug.WriteLine("3");
            var channelPointers = JsonConvert.DeserializeObject<IntPtr[]>(jsonPointers);
            Debug.WriteLine("4");
            var returnChannels = new List<Channel>();
            if (channelPointers == null)
            {
                return returnChannels;
            }

            foreach (var channelPointer in channelPointers)
            {
                var id = Channel.GetChannelIdFromPtr(channelPointer);
                if (TryGetChannel(id, channelPointer, out var channel))
                {
                    returnChannels.Add(channel);
                }
            }

            return returnChannels;
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
            channelWrappers.TryAdd(channelId, new Channel(this, channelId, newPointer));
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
        public void SetRestriction(string userId, string channelId, bool banUser, bool muteUser, string reason)
        {
            CUtilities.CheckCFunctionResult(
                pn_chat_set_restrictions(chatPointer, userId, channelId, banUser, muteUser, reason));
        }

        public void SetRestriction(string userId, string channelId, Restriction restriction)
        {
            SetRestriction(userId, channelId, restriction.Ban, restriction.Mute, restriction.Reason);
        }

        public void AddListenerToUsersUpdate(List<string> userIds, Action<User> listener)
        {
            foreach (var userId in userIds)
            {
                if (TryGetUser(userId, out var user))
                {
                    user.OnUserUpdated += listener;
                }
            }
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

        /// <summary>
        /// Checks if the user with the provided user ID is present in the provided channel.
        /// <para>
        /// Checks if the user with the provided user ID is present in the provided channel.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <param name="channelId">The channel ID.</param>
        /// <returns>True if the user is present, false otherwise.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// if (chat.IsPresent("user_id", "channel_id")) {
        ///   // User is present 
        /// }
        /// </code>
        /// </example>
        /// <seealso cref="WhoIsPresent"/>
        /// <seealso cref="WherePresent"/>
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

        /// <summary>
        /// Gets the list of users present in the provided channel.
        /// <para>
        /// Gets all the users as a list of the strings present in the provided channel.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <returns>The list of the users present in the channel.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var users = chat.WhoIsPresent("channel_id");
        /// foreach (var user in users) {
        ///   // User is present on the channel
        /// }
        /// </code>
        /// </example>
        /// <seealso cref="WherePresent"/>
        /// <seealso cref="IsPresent"/>
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

        /// <summary>
        /// Gets the list of channels where the user with the provided user ID is present.
        /// <para>
        /// Gets all the channels as a list of the strings where the user with the provided user ID is present.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <returns>The list of the channels where the user is present.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var channels = chat.WherePresent("user_id");
        /// foreach (var channel in channels) {
        ///  // Channel where User is IsPresent
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="WhoIsPresent"/>
        /// <seealso cref="IsPresent"/>
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

        /// <summary>
        /// Gets the user with the provided user ID.
        /// <para>
        /// Tries to get the user with the provided user ID.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <param name="user">The out user.</param>
        /// <returns>True if the user was found, false otherwise.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// if (chat.TryGetUser("user_id", out var user)) {
        ///   // User found
        /// }
        /// </code>
        /// </example>
        /// <seealso cref="User"/>
        public bool TryGetUser(string userId, out User user)
        {
            var userPointer = pn_chat_get_user(chatPointer, userId);
            return TryGetUser(userId, userPointer, out user);
        }

        private bool TryGetUser(string userId, IntPtr userPointer, out User user)
        {
            return TryGetWrapper(userWrappers, userId, userPointer, () => new User(this, userId, userPointer),
                out user);
        }

        /// <summary>
        /// Gets the list of users with the provided parameters.
        /// <para>
        /// Gets all the users that matches the provided parameters.
        /// </para>
        /// </summary>
        /// <param name="include">The include parameter.</param>
        /// <param name="limit">The amount of userts to get.</param>
        /// <param name="startTimeToken">The start time token of the users.</param>
        /// <param name="endTimeToken">The end time token of the users.</param>
        /// <returns>The list of the users that matches the provided parameters.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var users = chat.GetUsers(
        ///     "admin",
        ///     10,
        ///     "16686902600029072"
        ///     "16686902600028961",
        /// );
        /// foreach (var user in users) {
        ///  // User found
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="User"/>
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

        /// <summary>
        /// Updates the user with the provided user ID.
        /// <para>
        /// Updates the user with the provided user ID with the provided data.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <param name="updatedData">The updated data for the user.</param>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the user with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// chat.UpdateUser("user_id", new ChatUserData {
        ///   Username = "new_name"
        ///   // ...
        /// });
        /// </code>
        /// </example>
        /// <seealso cref="ChatUserData"/>
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
            userWrappers.TryAdd(userId, new User(this, userId, newPointer));
        }

        /// <summary>
        /// Deletes the user with the provided user ID.
        /// <para>
        /// The user is deleted with all the messages and channels.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the user with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// chat.DeleteUser("user_id");
        /// </code>
        /// </example>
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

        /// <summary>
        /// Gets the memberships of the user with the provided user ID.
        /// <para>
        /// Gets all the memberships of the user with the provided user ID.
        /// The memberships are limited by the provided limit and the time tokens.
        /// </para>
        /// </summary>
        /// <param name="userId">The user ID.</param>
        /// <param name="limit">The maximum amount of the memberships.</param>
        /// <param name="startTimeToken">The start time token of the memberships.</param>
        /// <param name="endTimeToken">The end time token of the memberships.</param>
        /// <returns>The list of the memberships of the user.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the user with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var memberships = chat.GetUserMemberships(
        ///         "user_id",
        ///         10,
        ///         "16686902600029072",
        ///         "16686902600028961"
        /// );
        /// foreach (var membership in memberships) {
        ///  // Membership found
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="Membership"/>
        public List<Membership> GetUserMemberships(string userId, int limit, string startTimeToken, string endTimeToken)
        {
            if (!TryGetUser(userId, out var user))
            {
                return new List<Membership>();
            }

            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_user_get_memberships(user.Pointer, limit, startTimeToken, endTimeToken,
                buffer));
            return ParseJsonMembershipPointers(buffer.ToString());
        }

        public void AddListenerToMembershipsUpdate(List<string> membershipIds, Action<Membership> listener)
        {
            foreach (var membershipId in membershipIds)
            {
                if (membershipWrappers.TryGetValue(membershipId, out var membership))
                {
                    membership.OnMembershipUpdated += listener;
                }
            }
        }

        /// <summary>
        /// Gets the memberships of the channel with the provided channel ID.
        /// <para>
        /// Gets all the memberships of the channel with the provided channel ID.
        /// The memberships are limited by the provided limit and the time tokens.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="limit">The maximum amount of the memberships.</param>
        /// <param name="startTimeToken">The start time token of the memberships.</param>
        /// <param name="endTimeToken">The end time token of the memberships.</param>
        /// <returns>The list of the memberships of the channel.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the channel with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var memberships = chat.GetChannelMemberships(
        ///         "user_id",
        ///         10,
        ///         "16686902600029072",
        ///         "16686902600028961"
        /// );
        /// foreach (var membership in memberships) {
        ///  // Membership found
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="Membership"/>
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

        internal List<Membership> ParseJsonMembershipPointers(string membershipPointersJson)
        {
            var memberships = new List<Membership>();
            if (CUtilities.IsValidJson(membershipPointersJson))
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

        private bool TryGetMembership(IntPtr membershipPointer, out Membership membership)
        {
            var membershipId = Membership.GetMembershipIdFromPtr(membershipPointer);
            return TryGetMembership(membershipId, membershipPointer, out membership);
        }

        internal bool TryGetMembership(string membershipId, IntPtr membershipPointer, out Membership membership)
        {
            return TryGetWrapper(membershipWrappers, membershipId, membershipPointer,
                () => new Membership(membershipPointer, membershipId), out membership);
        }

        #endregion

        #region Messages

        /// <summary>
        /// Gets the <c>Message</c> object for the given timetoken.
        /// <para>
        /// Gets the <c>Message</c> object from the channel for the given timetoken.
        /// The timetoken is used to identify the message.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="messageTimeToken">The timetoken of the message.</param>
        /// <param name="message">The out parameter that contains the <c>Message</c> object.</param>
        /// <returns><c>true</c> if the message is found; otherwise, <c>false</c>.</returns>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// if (chat.TryGetMessage("channel_id", "timetoken", out var message)) {
        ///  // Message found
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="Message"/>
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
            return TryGetWrapper(messageWrappers, timeToken, messagePointer,
                () => new Message(this, messagePointer, timeToken), out message);
        }
        
        internal bool TryGetMessage(IntPtr messagePointer, out Message message)
        {
            var messageId = Message.GetMessageIdFromPtr(messagePointer);
            return TryGetMessage(messageId, messagePointer, out message);
        }

        public void ForwardMessage(Message message, Channel channel)
        {
            CUtilities.CheckCFunctionResult(pn_chat_forward_message(chatPointer, message.Pointer, channel.Pointer));
        }

        public List<(Channel channel, Membership membership, int count)> GetUnreadMessageCounts(string startTimeToken,
            string endTimeToken, string filter, int limit)
        {
            throw new NotImplementedException();
        }

        public void AddListenerToMessagesUpdate(string channelId, List<string> messageTimeTokens,
            Action<Message> listener)
        {
            foreach (var messageTimeToken in messageTimeTokens)
            {
                if (TryGetMessage(channelId, messageTimeToken, out var message))
                {
                    message.OnMessageUpdated += listener;
                }
            }
        }

        public void PinMessageToChannel(string channelId, Message message)
        {
            if (TryGetChannel(channelId, out var channel))
            {
                channel.PinMessage(message);
            }
        }

        public void UnpinMessageFromChannel(string channelId)
        {
            if (TryGetChannel(channelId, out var channel))
            {
                channel.UnpinMessage();
            }
        }

        /// <summary>
        /// Gets the channel message history.
        /// <para>
        /// Gets the list of the messages that were sent in the channel with the provided parameters.
        /// The history is limited by the provided count of messages, start time token, and end time token.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <param name="startTimeToken">The start time token of the messages.</param>
        /// <param name="endTimeToken">The end time token of the messages.</param>
        /// <param name="count">The maximum amount of the messages.</param>
        /// <returns>The list of the messages that were sent in the channel.</returns>
        /// <exception cref="PubNubCCoreException"> Throws an exception if the channel with the provided ID does not exist or any connection problem persists.</exception>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// var messages = chat.GetChannelMessageHistory("channel_id", "start_time_token", "end_time_token", 10);
        /// foreach (var message in messages) {
        ///  // Message found
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="Message"/>
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

        //TODO: use enum instead of all these methods?
        #region Events

        //TODO: full summary
        /// <summary>
        /// Start listening for Moderation Events for this UserId.
        /// </summary>
        /// <param name="userId">User whose Moderation Events are to be broadcast.</param>
        public void StartListeningForUserModerationEvents(string userId)
        {
            ListenForEvents(userId);
        }

        //TODO: full summary
        //TODO: might turn this into a StartListeningForEvents<T> later
        /// <summary>
        /// Start listening for Report Events.
        /// </summary>
        public void StartListeningForReportEvents()
        {
            ListenForEvents("PUBNUB_INTERNAL_ADMIN_CHANNEL");
        }

        //TODO: I added that here but probably it is not the best place:
        /// <summary>
        /// Starts listening for events.
        /// <para>
        /// Starts listening for channel events. 
        /// It allows to receive different events without the need to 
        /// connect to any channel.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel ID.</param>
        /// <example>
        /// <code>
        /// var chat = // ...
        /// chat.ListenForEvents();
        /// chat.OnEvent += (event) => {
        ///  // Event received
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="OnEvent"/>
        internal void ListenForEvents(string channelId)
        {
            if (string.IsNullOrEmpty(channelId))
            {
                throw new ArgumentException("Channel ID cannot be null or empty.");
            }
            CUtilities.CheckCFunctionResult(pn_chat_listen_for_events(chatPointer, channelId));
            ParseJsonUpdatePointers(GetUpdates());
        }

        #endregion

        private bool TryGetWrapper<T>(Dictionary<string, T> wrappers, string id, IntPtr pointer, Func<T> createWrapper,
            out T wrapper) where T : PointerWrapper
        {
            if (wrappers.TryGetValue(id, out wrapper))
            {
                //We had it before but it's no longer valid
                if (pointer == IntPtr.Zero)
                {
                    Debug.WriteLine(CUtilities.GetErrorMessage());
                    wrappers.Remove(id);
                    return false;
                }

                //Pointer is valid but something nulled the wrapper
                if (wrapper == null)
                {
                    wrappers[id] = createWrapper();
                    wrapper = wrappers[id];
                    return true;
                }
                
                return true;
            }
            //Adding new user to wrappers cache
            else if (pointer != IntPtr.Zero)
            {
                wrapper = createWrapper();
                wrappers.Add(id, wrapper);
                return true;
            }
            else
            {
                Debug.WriteLine(CUtilities.GetErrorMessage());
                return false;
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
