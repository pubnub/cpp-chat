using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    /// <summary>
    /// Represents a user in the chat. 
    /// <para>
    /// You can get information about the user, update the user's data, delete the user, set restrictions on the user,
    /// </para>
    /// </summary>
    public class User : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_user_destroy(IntPtr user);

        [DllImport("pubnub-chat")]
        private static extern int pn_user_report(IntPtr user, string reason);

        [DllImport("pubnub-chat")]
        private static extern int pn_user_is_present_on(IntPtr user, string channel_id);

        [DllImport("pubnub-chat")]
        private static extern int pn_user_where_present(IntPtr user, StringBuilder result_json);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_user_id(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_user_name(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_external_id(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_profile_url(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_email(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_custom_data(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_status(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_user_get_data_type(IntPtr user, StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern int pn_user_get_channel_restrictions(
            IntPtr user,
            string user_id,
            string channel_id,
            int limit,
            string start,
            string end,
            StringBuilder result);

        #endregion

        /// <summary>
        /// The user's user name. 
        /// <para>
        /// This might be user's display name in the chat.
        /// </para>
        /// </summary>
        public string UserName
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_user_name(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user's external id.
        /// <para>
        /// This might be user's id in the external system (e.g. Database, CRM, etc.)
        /// </para>
        /// </summary>
        public string ExternalId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_external_id(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user's profile url.
        /// <para>
        /// This might be user's profile url to download the profile picture.
        /// </para>
        /// </summary>
        public string ProfileUrl
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_profile_url(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user's email.
        /// <para>
        /// This should be user's email address.
        /// </para>
        /// </summary>
        public string Email
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_email(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user's custom data.
        /// <para>
        /// This might be any custom data that you want to store for the user.
        /// </para>
        /// </summary>
        public string CustomData
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_custom_data(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user's status.
        /// <para>
        /// This is a string that represents the user's status.
        /// </para>
        /// </summary>
        public string Status
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_status(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// The user's data type.
        /// <para>
        /// This is a string that represents the user's data type.
        /// </para>
        /// </summary>
        public string DataType
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_type(pointer, buffer);
                return buffer.ToString();
            }
        }

        private Chat chat;

        /// <summary>
        /// Event that is triggered when the user is updated.
        /// <para>
        /// This event is triggered when the user's data is updated.
        /// You can subscribe to this event to get notified when the user is updated.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// // var user = // ...;
        /// user.OnUserUpdated += (user) =>
        /// {
        ///    Console.WriteLine($"User {user.UserName} is updated.");
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="UpdateUser"/>
        /// <seealso cref="User"/>
        public event Action<User> OnUserUpdated;

        internal User(Chat chat, string userId, IntPtr userPointer) : base(userPointer, userId)
        {
            this.chat = chat;
        }

        internal static string GetUserIdFromPtr(IntPtr userPointer)
        {
            var buffer = new StringBuilder(512);
            pn_user_get_user_id(userPointer, buffer);
            return buffer.ToString();
        }

        internal void BroadcastUserUpdate()
        {
            OnUserUpdated?.Invoke(this);
        }

        /// <summary>
        /// Updates the user.
        /// <para>
        /// This method updates the user's data.
        /// </para>
        /// </summary>
        /// <param name="updatedData">The updated data for the user.</param>
        /// <exception cref="PubNubCCoreException">
        /// This exception might be thrown when any error occurs while updating the user.
        /// </exception>
        /// <example>
        /// <code>
        /// var user = // ...;
        /// user.UpdateUser(new ChatUserData
        /// {
        ///    UserName = "New User Name",
        /// });
        /// </code>
        /// </example>
        /// <seealso cref="ChatUserData"/>
        public void UpdateUser(ChatUserData updatedData)
        {
            chat.UpdateUser(Id, updatedData);
        }

        /// <summary>
        /// Deletes the user.
        /// <para>
        /// This method deletes the user from the chat.
        /// It will remove the user from all the channels and delete the user's data.
        /// </para>
        /// </summary>
        /// <exception cref="PubNubCCoreException">
        /// This exception might be thrown when any error occurs while deleting the user.
        /// </exception>
        /// <example>
        /// <code>
        /// var user = // ...;
        /// user.DeleteUser();
        /// </code>
        /// </example>
        public void DeleteUser()
        {
            chat.DeleteUser(Id);
        }

        /// <summary>
        /// Sets restrictions on the user.
        /// <para>
        /// This method sets the restrictions on the user.
        /// You can ban the user from a channel, mute the user on the channel, or set the restrictions on the user.
        /// </para>
        /// </summary>
        /// <param name="channelId">The channel id on which the restrictions are set.</param>
        /// <param name="banUser">If set to <c>true</c>, the user is banned from the channel.</param>
        /// <param name="muteUser">If set to <c>true</c>, the user is muted on the channel.</param>
        /// <param name="reason">The reason for setting the restrictions on the user.</param>
        /// <exception cref="PubNubCCoreException">
        /// This exception might be thrown when any error occurs while setting the restrictions on the user.
        /// </exception>
        /// <example>
        /// <code>
        /// var user = // ...;
        /// user.SetRestrictions("channel_id", true, false, "Banned from the channel");
        /// </code>
        /// </example>
        public void SetRestrictions(string channelId, bool banUser, bool muteUser, string reason)
        {
            chat.SetRestrictions(Id, channelId, banUser, muteUser, reason);
        }

       public string GetChannelRestrictions(string channelId, int limit, string startTimeToken, string endTimeToken)
        {
            var buffer = new StringBuilder(8192);
            CUtilities.CheckCFunctionResult(pn_user_get_channel_restrictions(pointer, Id, channelId, limit,
                startTimeToken, endTimeToken, buffer));
            return buffer.ToString();
        }

        public void ReportUser(string reason)
        {
            CUtilities.CheckCFunctionResult(pn_user_report(pointer, reason));
        }

        public bool IsPresentOn(string channelId)
        {
            var result = pn_user_is_present_on(pointer, channelId);
            CUtilities.CheckCFunctionResult(result);
            return result == 1;
        }

        public List<string> WherePresent()
        {
            var buffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_user_where_present(pointer, buffer));
            var jsonChannelIds = buffer.ToString();
            var channelIds = JsonConvert.DeserializeObject<List<string>>(jsonChannelIds);
            channelIds ??= new List<string>();
            return channelIds;
        }

        public List<Membership> GetMemberships(int limit, string startTimeToken, string endTimeToken)
        {
            return chat.GetUserMemberships(Id, limit, startTimeToken, endTimeToken);
        }

        protected override void DisposePointer()
        {
            pn_user_destroy(pointer);
        }
    }
}
