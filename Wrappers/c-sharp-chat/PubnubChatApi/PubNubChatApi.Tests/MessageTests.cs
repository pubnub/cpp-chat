using PubNubChatAPI.Entities;

namespace PubNubChatApi.Tests;

public class MessageTests
{
    private Chat chat;
    private Channel channel;

    [SetUp]
    public void Setup()
    {
        chat = new Chat(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "message_tests_user");
        channel = chat.CreatePublicConversation("message_tests_channel");
        channel.Join();
    }

    [Test]
    public async Task TestSendAndReceive()
    {
        channel.OnMessageReceived += message =>
        {
            Assert.True(message.MessageText == "Test message text");
        };
        channel.SendText("Test message text");

        await Task.Delay(3000);
    }

    [Test]
    public async Task TestTryGetMessage()
    {
        channel.OnMessageReceived += message =>
        {
            if (message.ChannelId == channel.Id)
            {
                Assert.True(chat.TryGetMessage(channel.Id, message.TimeToken, out _));
            }
        };
        channel.SendText("something");

        await Task.Delay(3000);
    }
    
    [Test]
    public async Task TestEditMessage()
    {
        channel.OnMessageReceived += message =>
        {
            message.EditMessageText("new-text");
            message.OnMessageUpdated += updatedMessage =>
            {
                Assert.True(updatedMessage.MessageText == "new-text");
            };
        };
        channel.SendText("something");

        await Task.Delay(5000);
    }
    
    [Test]
    public async Task TestDeleteMessage()
    {
        channel.OnMessageReceived += async message =>
        {
            message.DeleteMessage();

            await Task.Delay(2000);
            
            Assert.True(message.IsDeleted);
        };
        channel.SendText("something");

        await Task.Delay(7000);
    }

    [Test]
    public async Task TestPinMessage()
    {
        channel.OnMessageReceived += async message =>
        {
            message.PinMessage();

            await Task.Delay(2000);
            
            Console.WriteLine("lol");
            Assert.True(channel.TryGetPinnedMessage(out var pinnedMessage) && pinnedMessage.MessageText == "message to pin");
        };
        channel.SendText("message to pin");
        
        await Task.Delay(6000);
    }
}