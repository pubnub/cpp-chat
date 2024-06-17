using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
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

        public string UserName
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_user_name(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string ExternalId
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_external_id(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string ProfileUrl
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_profile_url(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string Email
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_email(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string CustomData
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_custom_data(pointer, buffer);
                return buffer.ToString();
            }
        }

        public string Status
        {
            get
            {
                var buffer = new StringBuilder(512);
                pn_user_get_data_status(pointer, buffer);
                return buffer.ToString();
            }
        }

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