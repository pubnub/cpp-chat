using System;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class User
    {
        #region DLL Imports

        [DllImport("pubnub-chat.dll")]
        private static extern void pn_user_destroy(IntPtr user);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_user_report(IntPtr user, string reason);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_user_is_present_on(IntPtr user, string channel_id);
        [DllImport("pubnub-chat.dll")]
        private static extern int pn_user_where_present(IntPtr user, StringBuilder result_json);

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
            CUtilities.CheckCFunctionResult(pn_user_report(userPointer, reason));
        }

        public bool IsPresentOn(string channelId)
        {
            var result = pn_user_is_present_on(userPointer, channelId);
            CUtilities.CheckCFunctionResult(result);
            return result == 1;
        }

        public string WherePresent()
        {
            var buffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_user_where_present(userPointer, buffer));
            return buffer.ToString();
        }

        ~User()
        {
            pn_user_destroy(userPointer);
        }
    }
}