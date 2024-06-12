using System;
using System.Runtime.InteropServices;
using System.Text;

namespace PubnubChatApi.Utilities
{
    public class PubNubCCoreException : Exception
    {
        public PubNubCCoreException(string message) : base(message)
        {
        }
    }
    
    public static class CUtilities
    {
        [DllImport("pubnub-chat.dll")]
        private static extern void pn_c_get_error_message(StringBuilder buffer);

        private static void ThrowCError()
        {
            var buffer = new StringBuilder(4096);
            pn_c_get_error_message(buffer);
            throw new PubNubCCoreException(buffer.ToString());
        }

        internal static void CheckCFunctionResult(int result)
        {
            if (result == -1)
            {
                ThrowCError();
            }
        }

        internal static void CheckCFunctionResult(IntPtr result)
        {
            if (result == IntPtr.Zero)
            {
                ThrowCError();
            }
        }
    }
}