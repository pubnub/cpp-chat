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
    public void TestSendAndReceive()
    {
        var manualReceiveEvent = new ManualResetEvent(false);
        
        channel.OnMessageReceived += message =>
        {
            Assert.True(message.MessageText == "Test message text");
            manualReceiveEvent.Set();
        };
        channel.SendText("Test message text");
        
        var received = manualReceiveEvent.WaitOne(4000);
        Assert.IsTrue(received);
    }

    [Test]
    public void TestTryGetMessage()
    {
        var manualReceiveEvent = new ManualResetEvent(false);
        channel.OnMessageReceived += message =>
        {
            if (message.ChannelId == channel.Id)
            {
                Assert.True(chat.TryGetMessage(channel.Id, message.TimeToken, out _));
                manualReceiveEvent.Set();
            }
        };
        channel.SendText("something");

        var received = manualReceiveEvent.WaitOne(4000);
        Assert.IsTrue(received);
    }
    
    [Test]
    public void TestEditMessage()
    {
        var manualUpdatedEvent = new ManualResetEvent(false);
        channel.OnMessageReceived += message =>
        {
            message.EditMessageText("new-text");
            message.OnMessageUpdated += updatedMessage =>
            {
                manualUpdatedEvent.Set();
                Assert.True(updatedMessage.MessageText == "new-text");
            };
        };
        channel.SendText("something");

        var receivedAndUpdated = manualUpdatedEvent.WaitOne(4000);
        Assert.IsTrue(receivedAndUpdated);
    }
    
    [Test]
    public void TestDeleteMessage()
    {
        var manualReceivedEvent = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            message.Delete();

            await Task.Delay(2000);
            
            Assert.True(message.IsDeleted);
            manualReceivedEvent.Set();
        };
        channel.SendText("something");

        var received = manualReceivedEvent.WaitOne(4000);
        Assert.IsTrue(received);
    }

    [Test]
    public void TestPinMessage()
    {
        var manualReceivedEvent = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            message.Pin();

            await Task.Delay(2000);
            
            Console.WriteLine("lol");
            Assert.True(channel.TryGetPinnedMessage(out var pinnedMessage) && pinnedMessage.MessageText == "message to pin");
            manualReceivedEvent.Set();
        };
        channel.SendText("message to pin");

        var received = manualReceivedEvent.WaitOne(4000);
        Assert.IsTrue(received);
    }
}