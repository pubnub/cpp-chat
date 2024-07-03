using System.Diagnostics;
using PubNubChatAPI.Entities;
using PubnubChatApi.Entities.Data;

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

    [Test]
    public async Task TestPinMessage()
    {
        var channel = chat.CreatePublicConversation("pin_message_test_channel_26", new ChatChannelData()
        {
            ChannelName = "Some fucking name",
            ChannelCustomDataJson = "{}"
        });
        
        Debug.WriteLine($"Name on create: {channel.Name}");
        //Debug.WriteLine($"Json on create: {channel.CustomDataJson}");
        
        channel.Join();
        
        Debug.WriteLine($"Name after join: {channel.Name}");
        //Debug.WriteLine($"Json after join: {channel.CustomDataJson}");
        
        channel.OnMessageReceived += async message =>
        {
            Debug.WriteLine($"Name before pin: {channel.Name}");
            //Debug.WriteLine($"Json before pin: {channel.CustomDataJson}");
            
            channel.PinMessage(message);
            
            await Task.Delay(5000);
            
            Debug.WriteLine($"Name after pin: {channel.Name}");
            Debug.WriteLine($"Json after pin: {channel.CustomDataJson}");
            
            Assert.True(channel.TryGetPinnedMessage(out var pinnedMessage) && pinnedMessage.MessageText == "message to pin");
        };
        channel.SendText("message to pin");
        
        await Task.Delay(8000);
    }
    
    [Test]
    public async Task TestUnPinMessage()
    {
        var channel = chat.CreatePublicConversation("unpin_message_test_channel");
        channel.Join();
        channel.OnMessageReceived += async message =>
        {
            channel.PinMessage(message);

            await Task.Delay(2000);
            
            Assert.True(channel.TryGetPinnedMessage(out var pinnedMessage) && pinnedMessage.MessageText == "message to pin");
            channel.UnpinMessage();
            
            await Task.Delay(2000);
            
            Assert.False(channel.TryGetPinnedMessage(out _));
        };
        channel.SendText("message to pin");
        
        await Task.Delay(6000);
    }
}