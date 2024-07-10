using System;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    /// <summary>
    /// Represents a membership of a user in a channel.
    /// <para>
    /// Memberships are relations between users and channels. They are used to determine
    /// which users are allowed to send messages to which channels.
    /// </para>
    /// </summary>
    /// <remarks>
    /// Memberships are created when a user joins a channel and are deleted when a user leaves a channel.
    /// </remarks>
    /// <seealso cref="Chat"/>
    /// <seealso cref="User"/>
    /// <seealso cref="Channel"/>
    public class Membership : UniquePointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_membership_delete(IntPtr membership);

        [DllImport("pubnub-chat")]
        private static extern void pn_membership_get_user_id(
            IntPtr membership,
            StringBuilder result);

        [DllImport("pubnub-chat")]
        private static extern void pn_membership_get_channel_id(
            IntPtr membership,
            StringBuilder result);
        
        [DllImport("pubnub-chat")]
        private static extern IntPtr pn_membership_update_dirty(
            IntPtr membership,
            string custom_object_json);

        #endregion

        /// <summary>
        /// The user ID of the user that this membership belongs to.
        /// </summary>
        public string UserId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_membership_get_user_id(pointer, buffer);
                return buffer.ToString();
            }
        }
        
        /// <summary>
        /// The channel ID of the channel that this membership belongs to.
        /// </summary>
        public string ChannelId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_membership_get_channel_id(pointer, buffer);
                return buffer.ToString();
            }
        }

        /// <summary>
        /// Event that is triggered when the membership is updated.
        /// <para>
        /// This event is triggered when the membership is updated by the server.
        /// Every time the membership is updated, this event is triggered.
        /// </para>
        /// </summary>
        /// <example>
        /// <code>
        /// membership.OnMembershipUpdated += (membership) =>
        /// {
        ///    Console.WriteLine("Membership updated!");
        /// };
        /// </code>
        /// </example>
        /// <seealso cref="Update"/>
        public event Action<Membership> OnMembershipUpdated;

        internal Membership(IntPtr membershipPointer, string membershipId) : base(membershipPointer, membershipId)
        {
        }

        internal static string GetMembershipIdFromPtr(IntPtr membershipPointer)
        {
            var userIdBuffer = new StringBuilder(512);
            pn_membership_get_user_id(membershipPointer, userIdBuffer);
            var userId = userIdBuffer.ToString();
            var channelIdBuffer = new StringBuilder(512);
            pn_membership_get_channel_id(membershipPointer, channelIdBuffer);
            var channelId = channelIdBuffer.ToString();
            return userId + channelId;
        }

        internal void BroadcastMembershipUpdate()
        {
            OnMembershipUpdated?.Invoke(this);
        }

        /// <summary>
        /// Updates the membership with a custom JSON object.
        /// <para>
        /// This method updates the membership with a custom JSON object. This object can be used to store
        /// additional information about the membership.
        /// </para>
        /// </summary>
        /// <param name="customJsonObject">The custom JSON object to update the membership with.</param>
        /// <example>
        /// <code>
        /// membership.Update("{\"key\": \"value\"}");
        /// </code>
        /// </example>
        /// <seealso cref="OnMembershipUpdated"/>
        public void Update(string customJsonObject)
        {
            CUtilities.CheckCFunctionResult(pn_membership_update_dirty(pointer, customJsonObject));
        }
        
        public string GetLastReadMessageTimeToken()
        {
            throw new NotImplementedException();
        }
        
        public void SetLastReadMessage()
        {
            throw new NotImplementedException();
        }

        public int GetUnreadMessagesCount()
        {
            throw new NotImplementedException();
        }

        protected override void DisposePointer()
        {
            pn_membership_delete(pointer);
        }
    }
}
