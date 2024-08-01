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

    [Test]
    public void TestMessageReactions()
    {
        channel.Join();
        var manualReset = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            message.ToggleReaction("\"happy\"");

            await Task.Delay(3000);

            var has = message.HasUserReaction("\"happy\"");
            Assert.True(has);
            var reactions = message.Reactions(); 
            Assert.True(reactions.Count == 1 && reactions.Any(x => x.Value == "\"happy\""));
            manualReset.Set();
        };
        channel.SendText("a_message");
        var reacted = manualReset.WaitOne(7000);
        Assert.True(reacted);
    }
    
    [Test]
    public void TestMessageReport()
    {
        var reportManualEvent = new ManualResetEvent(false);
        channel.Join();
        chat.StartListeningForReportEvents();
        chat.OnReportEvent += reportEvent =>
        {
            Assert.True(reportEvent.Json.Contains("bad_message"));
            reportManualEvent.Set();
        };
        channel.OnMessageReceived += message =>
        {
            message.Report("bad_message");
        };
        channel.SendText("message_to_be_reported");
        var reported = reportManualEvent.WaitOne(12000);
        Assert.True(reported);
    }

    [Test]
    public void TestCreateThread()
    {
        var manualReceiveEvent = new ManualResetEvent(false);
        channel.OnMessageReceived += message =>
        {
            try
            {
                var thread = message.CreateThread();
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                Assert.Fail();
            }
            manualReceiveEvent.Set();
        };
        channel.SendText("thread_start_message");
        
        var received = manualReceiveEvent.WaitOne(5000);
        Assert.IsTrue(received);
    }
}