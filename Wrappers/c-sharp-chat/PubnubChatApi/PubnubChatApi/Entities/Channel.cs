using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Timers;
using Newtonsoft.Json;
using PubnubChatApi.Entities.Data;
using PubnubChatApi.Enums;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    /// <summary>
    /// Class <c>Channel</c> represents a chat channel.
    ///
    /// <para>
    /// A channel is a entity that allows users to publish and receive messages.
    /// </para>
    /// </summary>
    public class Channel : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_delete(IntPtr channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_connect(IntPtr channel, StringBuilder messages_json);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_disconnect(IntPtr channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_join(IntPtr channel, string additional_params,
            StringBuilder messages_json);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_leave(IntPtr channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_set_restrictions(IntPtr channel, string user_id, bool ban_user,
            bool mute_user, string reason);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_channel_id(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_send_text(IntPtr channel, string message, byte type, string metadata);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_get_user_restrictions(
            IntPtr channel,
            string user_id,
            string channel_id,
            int limit,
            string start,
            string end,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_data_channel_name(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_data_description(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_data_custom_data_json(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_data_updated(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_data_status(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_channel_get_data_type(
            IntPtr channel,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_is_present(IntPtr channel, string user_id);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_who_is_present(IntPtr channel, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_channel_invite_user(IntPtr channel, IntPtr user);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_invite_multiple(IntPtr channel, IntPtr[] users, int users_length,
            StringBuilder result_json);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_start_typing(IntPtr channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_channel_stop_typing(IntPtr channel);
        
        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_channel_pin_message(IntPtr channel, IntPtr message);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_channel_unpin_message(IntPtr channel);

        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_channel_get_pinned_message(IntPtr channel);
        
        #endregion

        /// <summary>
        /// The name of the channel.
        ///
        /// <para>
        /// The name of the channel that is human meaningful.
        /// </para>
        /// </summary>
        /// <value>The name of the channel.</value>
        public string Name
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_channel_name(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The description of the channel.
        ///
        /// <para>
        /// The description that allows users to understand the purpose of the channel.
        /// </para>
        public string Description
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_description(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The custom data of the channel.
        ///
        /// <para>
        /// The custom data that can be used to store additional information about the channel.
        /// </para>
        /// </summary>
        /// <remarks>
        /// The custom data is stored in JSON format.
        /// </remarks>
        public string CustomDataJson
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_custom_data_json(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The information about the last update of the channel.
        /// <para>
        /// The time when the channel was last updated.
        /// </para>
        /// </summary>
        public string Updated
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_updated(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The status of the channel.
        /// <para>
        /// The last status response received from the server.
        /// </para>
        /// </summary>
        public string Status
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_status(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The type of the channel.
        /// <para>
        /// The type of the response received from the server when the channel was created.
        /// </para>
        /// </summary>
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
        private Dictionary<string, Timer> typingIndicators = new();

        /// <summary>
        /// Event that is triggered when a message is received.
        ///
        /// <para>
        /// The event is triggered when a message is received in the channel 
        /// when the channel is connected.
        /// </para>
        /// </summary>
        /// <value>The event that is triggered when a message is received.</value>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.OnMessageReceived += (message) => {
        ///    Console.WriteLine($"Message received: {message.Text}");
        /// };
        /// channel.Connect();
        /// </code>
        /// </example>
        public event Action<Message> OnMessageReceived;

        /// <summary>
        /// Event that is triggered when the channel is updated.
        ///
        /// <para>
        /// The event is triggered when the channel is updated by the user 
        /// or by any other entity.
        /// </para>
        /// </summary>
        /// <value>The event that is triggered when the channel is updated.</value>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.OnChannelUpdate += (channel) => {
        ///   Console.WriteLine($"Channel updated: {channel.Name}");
        /// };
        /// channel.Connect();
        /// </code>
        /// </example>
        public event Action<Channel> OnChannelUpdate;

        /// <summary>
        /// Event that is triggered when any presence update occurs.
        ///
        /// <para>
        /// Presence update occurs when a user joins or leaves the channel.
        /// </para>
        /// </summary>
        /// <value>The event that is triggered when any presence update occurs.</value>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.OnPresenceUpdate += (users) => {
        ///   Console.WriteLine($"Users present: {string.Join(", ", users)}");
        /// };
        /// channel.Connect();
        /// </code>
        /// </example>
        ///
        public event Action<List<string>> OnPresenceUpdate;

        public event Action<List<string>> OnUsersTyping; 

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

        internal void ParseAndBroadcastTypingEvent(Dictionary<string, string> eventJson)
        {
            if (!eventJson.TryGetValue("userId", out var userId))
            {
                return;
            }
            if (!eventJson.TryGetValue("value", out var valueString) 
                || !bool.TryParse(valueString, out var typingValue))
            {
                return;
            }

            //stop typing
            if(!typingValue && typingIndicators.ContainsKey(userId))
            {
                typingIndicators[userId].Stop();
                typingIndicators.Remove(userId);
            }
            //start typing
            if(typingValue)
            {
                //Stop the old timer
                if(typingIndicators.TryGetValue(userId, out var typingTimer))
                {
                    typingTimer.Stop();
                }
    
                //Create and start new timer
                //TODO: Get this from config
                var newTimer = new Timer(5000);
                newTimer.Elapsed += (_, _) =>
                {
                    typingIndicators.Remove(userId);
                    OnUsersTyping?.Invoke(typingIndicators.Keys.ToList());
                };
                typingIndicators[userId] = newTimer;
                newTimer.Start();
            }
            OnUsersTyping?.Invoke(typingIndicators.Keys.ToList());
        }

        public void StartTyping()
        {
            CUtilities.CheckCFunctionResult(pn_channel_start_typing(pointer));
        }

        public void StopTyping()
        {
            CUtilities.CheckCFunctionResult(pn_channel_stop_typing(pointer));
        }

        public void PinMessage(Message message)
        {
            var newPointer = pn_channel_pin_message(pointer, message.Pointer);
            CUtilities.CheckCFunctionResult(newPointer);
            UpdatePointer(newPointer);
        }

        public void UnpinMessage()
        {
            var newPointer = pn_channel_unpin_message(pointer);
            CUtilities.CheckCFunctionResult(newPointer);
            UpdatePointer(newPointer);
        }

        //TODO: currently same result whether error or no pinned message present
        public bool TryGetPinnedMessage(out Message pinnedMessage)
        {
            var pinnedMessagePointer = pn_channel_get_pinned_message(pointer);
            if (pinnedMessagePointer != IntPtr.Zero)
            {
                return chat.TryGetMessage(pinnedMessagePointer, out pinnedMessage);
            }
            else
            {
                pinnedMessage = null;
                return false;
            }
        }

        /// <summary>
        /// Connects to the channel.
        /// <para>
        /// Connects to the channel and starts receiving messages. 
        /// After connecting, the <see cref="OnMessageReceived"/> event is triggered when a message is received.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.OnMessageReceived += (message) => {
        ///   Console.WriteLine($"Message received: {message.Text}");
        /// };
        /// channel.Connect();
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while connecting to the channel.</exception>
        /// <seealso cref="OnMessageReceived"/>
        /// <seealso cref="Disconnect"/>
        /// <seealso cref="Join"/>
        public void Connect()
        {
            connected = true;
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_connect(pointer, messagesBuffer));
            chat.ParseJsonUpdatePointers(messagesBuffer.ToString());
        }

        // TODO: Shouldn't join have additional parameters?
        /// <summary>
        /// Joins the channel.
        /// <para>
        /// Joins the channel and starts receiving messages.
        /// After joining, the <see cref="OnMessageReceived"/> event is triggered when a message is received.
        /// Additionally, there is a possibility to add additional parameters to the join request.
        /// It also adds the membership to the channel.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.OnMessageReceived += (message) => {
        ///  Console.WriteLine($"Message received: {message.Text}");
        /// };
        /// channel.Join();
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while joining the channel.</exception>
        /// <seealso cref="OnMessageReceived"/>
        /// <seealso cref="Connect"/>
        /// <seealso cref="Disconnect"/>
        public void Join()
        {
            connected = true;
            var messagesBuffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_channel_join(pointer, string.Empty, messagesBuffer));
            chat.ParseJsonUpdatePointers(messagesBuffer.ToString());
        }

        /// <summary>
        /// Disconnects from the channel.
        /// <para>
        /// Disconnects from the channel and stops receiving messages.
        /// Additionally, all the other listeners gets the presence update that the user has left the channel.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.Connect();
        /// //...
        /// channel.Disconnect();
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while disconnecting from the channel.</exception>
        /// <seealso cref="Connect"/>
        /// <seealso cref="Join"/>
        public void Disconnect()
        {
            connected = false;
            CUtilities.CheckCFunctionResult(pn_channel_disconnect(pointer));
        }

        /// <summary>
        /// Leaves the channel.
        /// <para>
        /// Leaves the channel and stops receiving messages.
        /// Additionally, all the other listeners gets the presence update that the user has left the channel.
        /// The membership is also removed from the channel.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.Join();
        /// //...
        /// channel.Leave();
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while leaving the channel.</exception>
        /// <seealso cref="Join"/>
        /// <seealso cref="Connect"/>
        /// <seealso cref="Disconnect"/>
        public void Leave()
        {
            connected = false;
            CUtilities.CheckCFunctionResult(pn_channel_leave(pointer));
        }

        /// <summary>
        /// Sets the restrictions for the user.
        /// <para>
        /// Sets the information about the restrictions for the user.
        /// The restrictions include banning and muting the user.
        /// </para>
        /// </summary>
        /// <param name="userId">The user identifier.</param>
        /// <param name="banUser">if set to <c>true</c> the user is banned.</param>
        /// <param name="muteUser">if set to <c>true</c> the user is muted.</param>
        /// <param name="reason">The reason for the restrictions.</param>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.SetRestrictions("user1", true, false, "Spamming");
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while setting the restrictions.</exception>
        /// <seealso cref="GetUserRestriction"/>
        public void SetRestrictions(string userId, bool banUser, bool muteUser, string reason)
        {
            CUtilities.CheckCFunctionResult(pn_channel_set_restrictions(pointer, userId, banUser, muteUser,
                reason));
        }

        public void SetRestrictions(string userId, Restriction restriction)
        {
            SetRestrictions(userId, restriction.Ban, restriction.Mute, restriction.Reason);
        }

        /// <summary>
        /// Sends the text message.
        /// <para>
        /// Sends the text message to the channel.
        /// The message is sent in the form of a text.
        /// </para>
        /// </summary>
        /// <param name="message">The message to be sent.</param>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.SendText("Hello, World!");
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while sending the message.</exception>
        /// <seealso cref="OnMessageReceived"/>
        public void SendText(string message)
        {
            CUtilities.CheckCFunctionResult(pn_channel_send_text(pointer, message,
                (byte)pubnub_chat_message_type.PCMT_TEXT, string.Empty));
        }

        /// <summary>
        /// Updates the channel.
        /// <para>
        /// Updates the channel with the new data.
        /// The data includes the name, description, custom data, and type of the channel.
        /// </para>
        /// </summary>
        /// <param name="updatedData">The updated data of the channel.</param>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.UpdateChannel(new ChatChannelData {
        ///  Name = "newName",
        ///  Description = "newDescription",
        ///  CustomDataJson = "{\"key\": \"value\"}",
        ///  Type = "newType"
        /// });
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while updating the channel.</exception>
        /// <seealso cref="OnChannelUpdate"/>
        /// <seealso cref="ChatChannelData"/>
        public void UpdateChannel(ChatChannelData updatedData)
        {
            chat.UpdateChannel(Id, updatedData);
        }

        /// <summary>
        /// Deletes the channel.
        /// <para>
        /// Deletes the channel and removes all the messages and memberships from the channel.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var channel = //...
        /// channel.DeleteChannel();
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while deleting the channel.</exception>
        public void DeleteChannel()
        {
            chat.DeleteChannel(Id);
        }

        /// <summary>
        /// Gets the user restrictions.
        /// <para>
        /// Gets the user restrictions that include the information about the bans and mutes.
        /// </para>
        /// </summary>
        /// <param name="userId">The user identifier.</param>
        /// <param name="limit">The maximum amount of the restrictions received.</param>
        /// <param name="startTimetoken">The start timetoken of the restrictions.</param>
        /// <param name="endTimetoken">The end timetoken of the restrictions.</param>
        /// <returns>The user restrictions in JSON format.</returns>
        /// <example>
        /// <code>
        /// var channel = //...
        /// var restrictions = channel.GetUserRestrictions(
        ///     "user1",
        ///     10,
        ///     "16686902600029072"
        ///     "16686902600028961",
        /// );
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while getting the user restrictions.</exception>
        /// <seealso cref="SetRestrictions"/>
        public Restriction GetUserRestriction(string userId, int limit, string startTimetoken, string endTimetoken)
        {
            var buffer = new StringBuilder(4096);
            CUtilities.CheckCFunctionResult(pn_channel_get_user_restrictions(pointer, userId, Id, limit, startTimetoken,
                endTimetoken, buffer));
            var restrictionJson = buffer.ToString();
            var restriction = new Restriction();
            if (CUtilities.IsValidJson(restrictionJson))
            {
                restriction = JsonConvert.DeserializeObject<Restriction>(restrictionJson);
            }

            return restriction;
        }

        /// <summary>
        /// Determines whether the user is present in the channel.
        /// <para>
        /// The method checks whether the user is present in the channel.
        /// </para>
        /// </summary>
        /// <param name="userId">The user identifier.</param>
        /// <returns><c>true</c> if the user is present in the channel; otherwise, <c>false</c>.</returns>
        /// <example>
        /// <code>
        /// var channel = //...
        /// var isUserPresent = channel.IsUserPresent("user1");
        /// Console.WriteLine($"User present: {isUserPresent}");
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while checking the presence of the user.</exception>
        /// <seealso cref="WhoIsPresent"/>
        public bool IsUserPresent(string userId)
        {
            var result = pn_channel_is_present(pointer, userId);
            CUtilities.CheckCFunctionResult(result);
            return result == 1;
        }

        /// <summary>
        /// Gets the list of users present in the channel.
        /// <para>
        /// Gets all the users that are present in the channel.
        /// </para>
        /// </summary>
        /// <returns>The list of users present in the channel.</returns>
        /// <example>
        /// <code>
        /// var channel = //...
        /// var users = channel.WhoIsPresent();
        /// foreach (var user in users) {
        ///  Console.WriteLine($"User present: {user}");
        /// }
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while getting the list of users present in the channel.</exception>
        /// <seealso cref="IsUserPresent"/>
        public List<string> WhoIsPresent()
        {
            var buffer = new StringBuilder(4096);
            CUtilities.CheckCFunctionResult(pn_channel_who_is_present(pointer, buffer));
            var jsonResult = buffer.ToString();
            var ret = new List<string>();
            if (CUtilities.IsValidJson(jsonResult))
            {
                ret = JsonConvert.DeserializeObject<List<string>>(jsonResult);
                ret ??= new List<string>();
            }

            return ret;
        }

        /// <summary>
        /// Gets the list of the <c>Membership</c> objects.
        /// <para>
        /// Gets the list of the <c>Membership</c> objects that represent the users that are members 
        /// of the channel and the relationships between the users and the channel.
        /// </para>
        /// </summary>
        /// <param name="limit">The maximum amount of the memberships received.</param>
        /// <param name="startTimeToken">The start timetoken of the memberships.</param>
        /// <param name="endTimeToken">The end timetoken of the memberships.</param>
        /// <returns>The list of the <c>Membership</c> objects.</returns>
        /// <example>
        /// <code>
        /// var channel = //...
        /// var memberships = channel.GetMemberships(10, "16686902600029072", "16686902600028961");
        /// foreach (var membership in memberships) {
        ///   Console.WriteLine($"Membership: {membership.UserId}");
        /// }
        /// </code>
        /// </example>
        /// <exception cref="PubnubCCoreException">Thrown when an error occurs while getting the list of memberships.</exception>
        /// <seealso cref="Membership"/>
        public List<Membership> GetMemberships(int limit, string startTimeToken, string endTimeToken)
        {
            return chat.GetChannelMemberships(Id, limit, startTimeToken, endTimeToken);
        }

        /// <summary>
        /// Gets the <c>Message</c> object for the given timetoken.
        /// <para>
        /// Gets the <c>Message</c> object for the given timetoken.
        /// The timetoken is used to identify the message.
        /// </para>
        /// </summary>
        /// <param name="timeToken">The timetoken of the message.</param>
        /// <param name="message">The out parameter that contains the <c>Message</c> object.</param>
        /// <returns><c>true</c> if the message is found; otherwise, <c>false</c>.</returns>
        /// <example>
        /// <code>
        /// var channel = //...
        /// if (channel.TryGetMessage("16686902600029072", out var message)) {
        ///  Console.WriteLine($"Message: {message.Text}");
        /// }
        /// </code>
        /// </example>
        /// <seealso cref="Message"/>
        public bool TryGetMessage(string timeToken, out Message message)
        {
            return chat.TryGetMessage(Id, timeToken, out message);
        }

        public List<Message> GetMessageHistory(string startTimeToken, string endTimeToken,
            int count)
        {
            return chat.GetChannelMessageHistory(Id, startTimeToken, endTimeToken, count);
        }

        public Membership Invite(User user)
        {
            var membershipPointer = pn_channel_invite_user(pointer, user.Pointer);
            CUtilities.CheckCFunctionResult(membershipPointer);
            var membershipId = Membership.GetMembershipIdFromPtr(membershipPointer);
            chat.TryGetMembership(membershipId, membershipPointer, out var membership);
            return membership;
        }

        public List<Membership> InviteMultiple(List<User> users)
        {
            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_channel_invite_multiple(pointer, users.Select(x => x.Pointer).ToArray(),
                users.Count, buffer));
            return chat.ParseJsonMembershipPointers(buffer.ToString());
        }

        protected override void DisposePointer()
        {
            pn_channel_delete(pointer);
        }
    }
}