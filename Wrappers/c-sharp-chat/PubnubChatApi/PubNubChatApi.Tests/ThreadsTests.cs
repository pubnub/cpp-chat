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
        channel = chat.CreatePublicConversation("threads_tests_channel");
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
}