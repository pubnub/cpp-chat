using System;
using System.Runtime.InteropServices;
using System.Text;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class User : PointerWrapper
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
        
        private Chat chat;
        public event Action<User> OnUserUpdated; 
        
        internal User(Chat chat, string userId, IntPtr userPointer) : base(userPointer, userId)
        {
            this.chat = chat;
        }

        internal static string GetUserIdFromPtr(IntPtr userPointer)
        {
            //TODO: C++ getters ID
            return string.Empty;
        }

        public void UpdateUser(ChatUserData updatedData)
        {
            chat.UpdateUser(Id, updatedData);
        }

        public void DeleteUser()
        {
            chat.DeleteUser(Id);
        }

        public void SetRestrictions(string channelId, bool banUser, bool muteUser, string reason)
        {
            chat.SetRestrictions(Id, channelId, banUser, muteUser, reason);
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

        public string WherePresent()
        {
            var buffer = new StringBuilder(32768);
            CUtilities.CheckCFunctionResult(pn_user_where_present(pointer, buffer));
            return buffer.ToString();
        }

        protected override void DisposePointer()
        {
            pn_user_destroy(pointer);
        }
    }
}