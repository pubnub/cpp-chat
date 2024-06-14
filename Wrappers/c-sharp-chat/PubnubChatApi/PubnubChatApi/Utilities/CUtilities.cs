using System;
using System.Diagnostics;
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
            Debug.WriteLine("Throwing C-side Error!");
            throw new PubNubCCoreException(GetErrorMessage());
        }

        internal static string GetErrorMessage()
        {
            var buffer = new StringBuilder(4096);
            pn_c_get_error_message(buffer);
            return buffer.ToString();
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