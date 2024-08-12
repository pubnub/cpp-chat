using PubNubChatAPI.Entities;
using PubnubChatApi.Entities.Data;

namespace PubNubChatApi.Tests;

public class ChatEventTests
{
    private Chat chat;
    private Channel channel;
    private User user;

    [SetUp]
    public void Setup()
    {
        chat = new Chat(new PubnubChatConfig(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "event_tests_user")
        );
        channel = chat.CreatePublicConversation("event_tests_channel");
        user = chat.CreateUser("event_tests_user");
        channel.Join();
    }
    
    //TODO: fully remove once 100% sure it's obsolete
    /*[Test]
    public void TestReportEvents()
    {
        var manualReportedEvent = new ManualResetEvent(false);
        chat.OnReportEvent += reportEvent =>
        {
            Assert.True(reportEvent.Payload.Contains("some_reason"));
            manualReportedEvent.Set();
        };
        chat.StartListeningForReportEvents(channel.Id);
        user.ReportUser("some_reason");
        var reported = manualReportedEvent.WaitOne(4000);
        Assert.IsTrue(reported);
    }*/
    
    [Test]
    public void TestModerationEvents()
    {
        var manualModerationEvent = new ManualResetEvent(false);
        chat.OnModerationEvent += moderationEvent =>
        {
            Assert.True(moderationEvent.Payload.Contains("some_reason"));
            manualModerationEvent.Set();
        };
        chat.StartListeningForModerationEvents(user.Id);
        user.SetRestriction(channel.Id, new Restriction()
        {
            Ban = true,
            Mute = true,
            Reason = "some_reason"
        });
        var moderationEventReceived = manualModerationEvent.WaitOne(5000);
        Assert.IsTrue(moderationEventReceived);
    }
}