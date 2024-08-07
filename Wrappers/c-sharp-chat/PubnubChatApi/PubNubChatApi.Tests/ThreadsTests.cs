using System.Diagnostics;
using PubNubChatAPI.Entities;

namespace PubNubChatApi.Tests;

public class ThreadsTests
{
    private Chat chat;
    private Channel channel;

    [SetUp]
    public void Setup()
    {
        chat = new Chat(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "threads_tests_user");
        channel = chat.CreatePublicConversation("threads_tests_channel_36");
        channel.Join();
    }

    [Test]
    public void TestGetThreadHistory()
    {
        var historyReadReset = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            var thread = message.CreateThread();
            thread.SendText("one");
            thread.SendText("two");
            thread.SendText("three");

            await Task.Delay(6000);

            var history = thread.GetThreadHistory("99999999999999999", "00000000000000000", 3);
            Assert.True(history.Count == 3 && history.Any(x => x.MessageText == "one"));
            historyReadReset.Set();
        };
        channel.SendText("thread_start_message");
        var read = historyReadReset.WaitOne(12000);
        Assert.True(read);
    }

    [Test]
    public void TestThreadChannelParentChannelPinning()
    {
        var historyReadReset = new ManualResetEvent(false);
        channel.OnMessageReceived += async message =>
        {
            var thread = message.CreateThread();
            thread.Join();
            thread.OnMessageReceived += threadMessage =>
            {
                thread.PinMessageToParentChannel(threadMessage);
            };
            thread.SendText("some_thread_message");
            
            await Task.Delay(5000);

            Assert.True(channel.TryGetPinnedMessage(out var pinnedMessage) && pinnedMessage.MessageText == "some_thread_message");
            thread.UnPinMessageFromParentChannel();
            
            await Task.Delay(5000);

            Assert.False(channel.TryGetPinnedMessage(out _));
            historyReadReset.Set();
        };
        channel.SendText("thread_start_message");
        var read = historyReadReset.WaitOne(15000);
        Assert.True(read);
    }
    
    [Test]
    public void TestThreadMessageParentChannelPinning()
    {
    }
}