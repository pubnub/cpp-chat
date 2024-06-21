using PubNubChatAPI.Entities;

namespace PubNubChatApi.Tests;

public class MembershipTests
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
            "membership_tests_user");
        channel = chat.CreatePublicConversation("membership_tests_channel");
        user = chat.CreateUser("membership_tests_user");
        channel.Join();
    }

    [Test]
    public async Task TestGetMemberships()
    {
        var memberships = user.GetMemberships(50, "99999999999999999", "00000000000000000");
        Assert.True(memberships.Any(x => x.ChannelId == channel.Id && x.UserId == user.Id));
    }
    
    [Test]
    public async Task TestUpdateMemberships()
    {
        var memberships = user.GetMemberships(50, "99999999999999999", "00000000000000000");
        var testMembership = memberships[0];
        testMembership.OnMembershipUpdated += membership =>
        {
            Assert.True(membership.Id == testMembership.Id);
        };
        testMembership.Update("{\"key\": \"value\"}");
        await Task.Delay(4000);
    }
}