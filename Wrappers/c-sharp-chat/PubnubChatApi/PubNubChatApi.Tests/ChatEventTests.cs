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
        chat = new Chat(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "event_tests_user");
        channel = chat.CreatePublicConversation("event_tests_channel");
        user = chat.CreateUser("event_tests_user");
        channel.Join();
    }
    
    [Test]
    public async Task TestReportEvents()
    {
        chat.OnReportEvent += reportEvent =>
        {
            Assert.True(reportEvent.Json.Contains("some_reason"));
        };
        chat.StartListeningForReportEvents();
        user.ReportUser("some_reason");
        await Task.Delay(3000);
    }
    
    [Test]
    public async Task TestModerationEvents()
    {
        chat.OnModerationEvent += moderationEvent =>
        {
            Assert.True(moderationEvent.Json.Contains("some_reason"));
        };
        chat.StartListeningForUserModerationEvents(user.Id);
        user.SetRestriction(channel.Id, new Restriction()
        {
            Ban = true,
            Mute = true,
            Reason = "some_reason"
        });
        await Task.Delay(3000);
    }
}