using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;
using PubnubChatApi.Utilities;

namespace PubNubChatAPI.Entities
{
    public class ThreadChannel : PointerWrapper
    {
        #region DLL Imports

        [DllImport("pubnub-chat")]
        private static extern void pn_thread_channel_dispose(
            IntPtr thread_channel);

        [DllImport("pubnub-chat")]
        private static extern int pn_thread_channel_get_history(
            IntPtr thread_channel,
            string start_timetoken,
            string end_timetoken,
            int count,
            StringBuilder thread_messages_pointers_json);

        #endregion

        internal ThreadChannel(IntPtr pointer) : base(pointer)
        {
        }

        public List<ThreadMessage> GetHistory(string startTimeToken, string endTimeToken, int count)
        {
            var buffer = new StringBuilder(4096);
            CUtilities.CheckCFunctionResult(pn_thread_channel_get_history(pointer, startTimeToken, endTimeToken, count, buffer));
            var messagesPointersJson = buffer.ToString();
            var history = new List<ThreadMessage>();
            if (!CUtilities.IsValidJson(messagesPointersJson))
            {
                return history;
            }
            var messagePointers = JsonConvert.DeserializeObject<IntPtr[]>(messagesPointersJson);
            if (messagePointers == null)
            {
                return history;
            }
            foreach (var messagePointer in messagePointers)
            {
                history.Add(new ThreadMessage(messagePointer));
            }
            return history;
        }

        protected override void DisposePointer()
        {
            pn_thread_channel_dispose(pointer);
        }
    }
}