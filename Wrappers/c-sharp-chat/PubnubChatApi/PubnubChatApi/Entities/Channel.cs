using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
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

        // TODO: Docs
        public string Updated
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_channel_get_data_updated(pointer, buffer);
                return buffer.ToString();
            }
        }

        // TODO: Docs
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
            OnMessageReceived = null;
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
            OnMessageReceived = null;
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
