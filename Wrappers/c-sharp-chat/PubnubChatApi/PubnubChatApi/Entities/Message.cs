using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Entities.Data;
using PubnubChatApi.Enums;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    /// <summary>
    /// Represents a message in a chat channel.
    /// <para>
    /// Messages are sent by users to chat channels. They can contain text
    /// and other data, such as metadata or message actions.
    /// </para>
    /// </summary>
    /// <seealso cref="Chat"/>
    /// <seealso cref="Channel"/>
    public class Message : UniquePointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_message_delete(IntPtr message);
        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_message_edit_text(IntPtr message, string text);
        [DllImport("pubnub-chat")]
        private static extern int pn_message_text(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern int pn_message_delete_message(IntPtr message);
        [DllImport("pubnub-chat")]
        private static extern int pn_message_deleted(IntPtr message);
        
        [DllImport("pubnub-chat")]
        private static extern int pn_message_get_timetoken(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern int pn_message_get_data_type(IntPtr message);
        [DllImport("pubnub-chat")]
        private static extern void pn_message_get_data_text(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern void pn_message_get_data_channel_id(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern void pn_message_get_data_user_id(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern void pn_message_get_data_meta(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern void pn_message_get_data_message_actions(IntPtr message, StringBuilder result);
        [DllImport("pubnub-chat")]
        private static extern int pn_message_pin(IntPtr message);

        #endregion

        /// <summary>
        /// The text content of the message.
        /// <para>
        /// This is the main content of the message. It can be any text that the user wants to send.
        /// </para>
        /// </summary>
        public string MessageText
        {
            get
            {
                var buffer = new StringBuilder(32768);
                CUtilities.CheckCFunctionResult(pn_message_text(pointer, buffer));
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The time token of the message.
        /// <para>
        /// The time token is a unique identifier for the message.
        /// It is used to identify the message in the chat.
        /// </para>
        /// </summary>
        public string TimeToken
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_message_get_timetoken(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The channel ID of the channel that the message belongs to.
        /// <para>
        /// This is the ID of the channel that the message was sent to.
        /// </para>
        /// </summary>
        public string ChannelId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_message_get_data_channel_id(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user ID of the user that sent the message.
        /// <para>
        /// This is the unique ID of the user that sent the message.
        /// Do not confuse this with the username of the user.
        /// </para>
        /// </summary>
        public string UserId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_message_get_data_user_id(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The metadata of the message.
        /// <para>
        /// The metadata is additional data that can be attached to the message.
        /// It can be used to store additional information about the message.
        /// </para>
        /// </summary>
        public string Meta
        {
            get
            {
                var buffer = new StringBuilder(4096);
                pn_message_get_data_meta(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// Whether the message has been deleted.
        /// <para>
        /// This property indicates whether the message has been deleted.
        /// If the message has been deleted, this property will be true.
        /// It means that all the deletions are soft deletions.
        /// </para>
        /// </summary>
        public bool IsDeleted
        {
            get
            {
                var result = pn_message_deleted(pointer);
                CUtilities.CheckCFunctionResult(result);
                return result == 1;
            }
        }

        //TODO: format to list? We have the struct for it now
        //TODO: REMOVE THAT
        public string MessageActions
        {
            get
            {
                var buffer = new StringBuilder(4096);
                pn_message_get_data_message_actions(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The data type of the message.
        /// <para>
        /// This is the type of the message data.
        /// It can be used to determine the type of the message.
        /// </para>
        /// </summary>
        /// <seealso cref="pubnub_chat_message_type"/>
        public PubnubChatMessageType Type => (PubnubChatMessageType)pn_message_get_data_type(pointer);
        
        private Chat chat;

        /// <summary>
        /// Event that is triggered when the message is updated.
        /// <para>
        /// This event is triggered when the message is updated by the server.
        /// Every time the message is updated, this event is triggered.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var message = // ...;
        /// message.OnMessageUpdated += (message) =>
        /// {
        ///   Console.WriteLine("Message updated!");
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="EditMessageText"/>
        /// <seealso cref="Delete"/>
        public event Action<Message> OnMessageUpdated; 
        
        internal Message(Chat chat, IntPtr messagePointer, string timeToken) : base(messagePointer, timeToken)
        {
            this.chat = chat;
        }
        
        internal static string GetMessageIdFromPtr(IntPtr messagePointer)
        {
            var buffer = new StringBuilder(512);
            pn_message_get_timetoken(messagePointer, buffer);
            return buffer.ToString();
        }
        
        internal static string GetChannelIdFromMessagePtr(IntPtr messagePointer)
        {
            var buffer = new StringBuilder(512);
            pn_message_get_data_channel_id(messagePointer, buffer);
            return buffer.ToString();
        }

        internal void BroadcastMessageUpdate()
        {
            OnMessageUpdated?.Invoke(this);
        }

        /// <summary>
        /// Edits the text of the message.
        /// <para>
        /// This method edits the text of the message.
        /// It changes the text of the message to the new text provided.
        /// </para>
        /// </summary>
        /// <param name="newText">The new text of the message.</param>
        /// <example>
        /// <code>
        /// var message = // ...;
        /// message.EditMessageText("New text");
        /// </code>
        /// </example>
        /// <seealso cref="OnMessageUpdated"/>
        public void EditMessageText(string newText)
        {
            var newPointer = pn_message_edit_text(pointer, newText);
            CUtilities.CheckCFunctionResult(newPointer);
        }

        public void Pin()
        {
            CUtilities.CheckCFunctionResult(pn_message_pin(pointer));
        }

        public void Report(string reason)
        {
            throw new NotImplementedException();
        }

        public void Forward(string channelId)
        {
            if (chat.TryGetChannel(channelId, out var channel))
            {
                chat.ForwardMessage(this, channel);
            }
        }

        public List<MessageAction> Reactions()
        {
            throw new NotImplementedException();
        }

        public bool HasUserReaction(string reactionValue)
        {
            throw new NotImplementedException();
        }
        
        public void ToggleReaction(string reactionValue)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Deletes the message.
        /// <para>
        /// This method deletes the message.
        /// It marks the message as deleted.
        /// It means that the message will not be visible to other users, but the 
        /// message is treated as soft deleted.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// var message = // ...;
        /// message.DeleteMessage();
        /// </code>
        /// </example>
        /// <seealso cref="IsDeleted"/>
        /// <seealso cref="OnMessageUpdated"/>
        public void Delete()
        {
            CUtilities.CheckCFunctionResult(pn_message_delete_message(pointer));
        }

        protected override void DisposePointer()
        {
            pn_message_delete(pointer);
        }
    }
}
