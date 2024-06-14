using System;
using System.Runtime.InteropServices;
using System.Text;

namespace PubNubChatAPI.Entities
{
    public class Membership : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_membership_delete(IntPtr membership);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_membership_get_user_id(
            IntPtr membership,
            StringBuilder result);

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_membership_get_channel_id(
            IntPtr membership,
            StringBuilder result);

        #endregion

        public event Action<Membership> OnMembershipUpdated;

        internal Membership(IntPtr membershipPointer, string membershipId) : base(membershipPointer, membershipId)
        {
        }

        internal static string GetMembershipIdFromPtr(IntPtr membershipPointer)
        {
            var buffer = new StringBuilder(512);
            pn_membership_get_user_id(membershipPointer, buffer);
            var userId = buffer.ToString();
            buffer = new StringBuilder(512);
            pn_membership_get_channel_id(membershipPointer, buffer);
            return userId + buffer;
        }

        internal void BroadcastMembershipUpdate()
        {
            OnMembershipUpdated?.Invoke(this);
        }

        protected override void DisposePointer()
        {
            pn_membership_delete(pointer);
        }
    }
}