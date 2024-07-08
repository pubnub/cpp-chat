using System;

namespace PubnubChatApi.Entities.Events
{
    public class ReportEvent : ChatEvent
    {
        internal ReportEvent(IntPtr pointer, string json) : base(pointer, json)
        {
        }

        protected override void DisposePointer()
        {
            throw new NotImplementedException();
        }
    }
}