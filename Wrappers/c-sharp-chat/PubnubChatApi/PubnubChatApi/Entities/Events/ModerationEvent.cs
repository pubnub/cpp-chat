using System;

namespace PubnubChatApi.Entities.Events
{
    public class ModerationEvent : ChatEvent
    {
        internal ModerationEvent(IntPtr pointer, string json) : base(pointer, json)
        {
        }

        protected override void DisposePointer()
        {
            throw new NotImplementedException();
        }
    }
}