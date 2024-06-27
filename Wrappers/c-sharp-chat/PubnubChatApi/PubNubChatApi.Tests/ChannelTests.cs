using PubNubChatAPI.Entities;

namespace PubNubChatApi.Tests;

public class ChannelTests
{
    private Chat chat;
    private User user;

    [SetUp]
    public void Setup()
    {
        chat = new Chat(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "channel_tests_user");
        user = chat.CreateUser("channel_tests_user");
    }

    [Test]
    public async Task TestStartTyping()
    {
        var channel = chat.CreatePublicConversation("start_typing_test_channel");
        channel.Join();
        channel.OnUsersTyping += typingUsers =>
        {
            Assert.That(typingUsers, Does.Contain(user.Id));
        };
        channel.StartTyping();

        await Task.Delay(3000);
    }
    
    [Test]
    public async Task TestStopTyping()
    {
        var channel = chat.CreatePublicConversation("stop_typing_test_channel");
        channel.Join();
        
        channel.StartTyping();
        
        await Task.Delay(1000);
        
        channel.OnUsersTyping += typingUsers =>
        {
            Assert.That(typingUsers, Is.Empty);
        };
        channel.StopTyping();

        await Task.Delay(3000);
    }
    
    [Test]
    public async Task TestStopTypingFromTimer()
    {
        var channel = chat.CreatePublicConversation("stop_typing_timer_test_channel");
        channel.Join();
        
        channel.StartTyping();

        await Task.Delay(3000);
        
        channel.OnUsersTyping += typingUsers =>
        {
            Assert.That(typingUsers, Is.Empty);
        };
        
        await Task.Delay(9000);
    }
}