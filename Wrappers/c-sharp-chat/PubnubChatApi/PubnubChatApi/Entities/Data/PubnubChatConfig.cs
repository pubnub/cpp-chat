namespace PubnubChatApi.Entities.Data
{
    public class PubnubChatConfig
    {
        public string PublishKey { get; }
        public string SubscribeKey { get; }
        public string UserId { get; }
        public string AuthKey { get; }

        public PubnubChatConfig(string publishKey, string subscribeKey, string userId, string authKey = "")
        {
            PublishKey = publishKey;
            SubscribeKey = subscribeKey;
            UserId = userId;
            AuthKey = authKey;
        }
    }
}