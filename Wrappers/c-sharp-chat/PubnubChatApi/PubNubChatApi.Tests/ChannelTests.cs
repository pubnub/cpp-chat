using System.Diagnostics;
using PubNubChatAPI.Entities;

namespace PubNubChatApi.Tests;

public class ChannelTests
{
    private Chat chat;
    private Channel channel;
    private User user;

    [SetUp]
    public void Setup()
    {
        chat = new Chat(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "channel_tests_user");
        channel = chat.CreatePublicConversation("channel_tests_channel");
        user = chat.CreateUser("channel_tests_user");
        channel.Join();
    }

    [Test]
    public async Task TestTypingEvent()
    {
        channel.OnUsersTyping += typingUsers =>
        {
            Debug.WriteLine($"TYPING EVENT COUNT: {typingUsers.Count}");
            foreach (var typingUser in typingUsers)
            {
                Debug.WriteLine($"{typingUser} is typing");
            }
        };
        channel.StartTyping();

        await Task.Delay(10000);
    }
}