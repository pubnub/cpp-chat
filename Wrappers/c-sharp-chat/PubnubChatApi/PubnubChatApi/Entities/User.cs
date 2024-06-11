using System;
using System.Runtime.InteropServices;

namespace PubNubChatAPI.Entities
{
    public class User
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_user_destroy(IntPtr user);
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_user_report(IntPtr user, string reason);
        [DllImport("pubnub-chat.dll")]
        private static extern bool pn_user_is_present_on(IntPtr user, string channel_id);

        #endregion
        
        public string UserId { get; }
        
        private IntPtr userPointer;
        private Chat chat;
        
        internal User(Chat chat, string userId, IntPtr userPointer)
        {
            this.chat = chat;
            UserId = userId;
            this.userPointer = userPointer;
        }

        public void UpdateUser(ChatUserData updatedData)
        {
            chat.UpdateUser(UserId, updatedData);
        }

        public void DeleteUser()
        {
            chat.DeleteUser(UserId);
        }

        public void SetRestrictions(string channelId, bool banUser, bool muteUser, string reason)
        {
            chat.SetRestrictions(UserId, channelId, banUser, muteUser, reason);
        }

        public void ReportUser(string reason)
        {
            pn_user_report(userPointer, reason);
        }

        public bool IsPresentOn(string channelId)
        {
            return pn_user_is_present_on(userPointer, channelId);
        }

        ~User()
        {
            pn_user_destroy(userPointer);
        }
    }
}