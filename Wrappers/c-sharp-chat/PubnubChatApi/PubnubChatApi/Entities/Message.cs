using System.Collections.Generic;
using PubnubChatApi.Enums;

namespace PubNubChatAPI.Entities
{
    //TODO: proper C# style field names and give the C++ ones in Newtonsoft attributes
    public struct ChatMessageData
    {
        public pubnub_chat_message_type type;
        public string text;
        public string channel_id;
        public string user_id;
        public string meta;
        public List<string> message_actions;
    }
    
    public class Message
    {
        private string timetoken;
        private ChatMessageData messageData;
        
        public Message(string timetoken, ChatMessageData messageData)
        {
            this.timetoken = timetoken;
            this.messageData = messageData;
        }
    }
}