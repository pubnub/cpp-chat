using System.Diagnostics;
using PubNubChatAPI.Entities;
using PubnubChatApi.Entities.Data;

namespace PubNubChatApi.Tests;

public class MessageTests
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
            "message_tests_user");
        channel = chat.CreatePublicConversation("message_tests_channel_2");
        user = chat.CreateUser("message_tests_user");
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
        channel.SendText("Test message text", new SendTextParams()
        {
            MentionedUsers = new Dictionary<int, User>(){{0, user}},
            ReferencedChannels = new Dictionary<int, Channel>(){{0, channel}},
            TextLinks =
            [
                new TextLink()
                {
                    StartIndex = 0,
                    EndIndex = 13,
                    Link = "www.google.com"
                }
            ]
        });
        
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
    public async Task TestPinMessage()
    {
        if (chat.TryGetChannel("pin_test_2", out var existingChannel))
        {
            chat.DeleteChannel(existingChannel.Id);
            await Task.Delay(4000);
        }
        var pinTestChannel = chat.CreatePublicConversation("pin_test_2");
        pinTestChannel.Join();
       
        var manualReceivedEvent = new ManualResetEvent(false);
        pinTestChannel.OnMessageReceived += async message =>
        {
            message.Pin();

            await Task.Delay(3000);

            var got = pinTestChannel.TryGetPinnedMessage(out var pinnedMessage);
            Debug.WriteLine(pinnedMessage.MessageText);
            Assert.True(got && pinnedMessage.MessageText == "message to pin");
            manualReceivedEvent.Set();
        };
        pinTestChannel.SendText("message to pin");

        var received = manualReceivedEvent.WaitOne(12000);
        Assert.IsTrue(received);
    }

    [Test]
    public void TestMessageReactions()
    {
        channel.Join();
        var manualReset = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            message.ToggleReaction("happy");

            await Task.Delay(3000);

            var has = message.HasUserReaction("happy");
            Assert.True(has);
            var reactions = message.Reactions(); 
            Assert.True(reactions.Count == 1 && reactions.Any(x => x.Value == "happy"));
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
        chat.StartListeningForReportEvents(channel.Id);
        chat.OnReportEvent += reportEvent =>
        {
            Assert.True(reportEvent.Payload.Contains("bad_message"));
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
    public async Task TestCreateThread()
    {
        var manualReceiveEvent = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            var hasThread = false;
            try
            {
                var thread = message.CreateThread();
                await Task.Delay(3000);
                thread.SendText("thread_init_text");
                await Task.Delay(3000);
                hasThread = message.HasThread();
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                Assert.Fail();
            }
            Assert.True(hasThread);
            Assert.True(message.TryGetThread(out var threadChannel));
            message.RemoveThread();

            await Task.Delay(4000);
            Assert.False(message.HasThread());
            
            manualReceiveEvent.Set();
        };
        channel.SendText("thread_start_message");
        
        var received = manualReceiveEvent.WaitOne(20000);
        Assert.IsTrue(received);
    }
}