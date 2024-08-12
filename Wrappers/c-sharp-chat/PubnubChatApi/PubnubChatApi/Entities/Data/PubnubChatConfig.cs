namespace PubnubChatApi.Entities.Data
{
    public class PubnubChatConfig
    {
        public string PublishKey { get; }
        public string SubscribeKey { get; }
        public string UserId { get; }
        public string SecretKey { get; }
        public string AuthKey { get; }

        public PubnubChatConfig(string publishKey, string subscribeKey, string userId, string secretKey = "", string authKey = "")
        {
            PublishKey = publishKey;
            SubscribeKey = subscribeKey;
            UserId = userId;
            SecretKey = secretKey;
            AuthKey = authKey;
        }
    }
}