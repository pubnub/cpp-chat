using System;
using System.Runtime.InteropServices;

namespace PubNubChatAPI.Entities
{
    public class Membership : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_membership_delete(IntPtr membership);

        #endregion

        public event Action<Membership> OnMembershipUpdated; 
        
        internal Membership(IntPtr membershipPointer, string membershipId) : base(membershipPointer, membershipId)
        {
        }
        
        internal static string GetMembershipIdFromPtr(IntPtr userPointer)
        {
            //TODO: C++ getters ID
            return string.Empty;
        }

        protected override void DisposePointer()
        {
            pn_membership_delete(pointer);
        }
    }
}