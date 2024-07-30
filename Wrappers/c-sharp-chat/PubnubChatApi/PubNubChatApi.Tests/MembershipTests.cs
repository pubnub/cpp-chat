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
    public void TestUpdateMemberships()
    {
        var memberships = user.GetMemberships(50, "99999999999999999", "00000000000000000");
        var testMembership = memberships[0];
        var manualUpdatedEvent = new ManualResetEvent(false);
        testMembership.OnMembershipUpdated += membership =>
        {
            Assert.True(membership.Id == testMembership.Id);
            manualUpdatedEvent.Set();
        };
        testMembership.Update("{\"key\": \"value\"}");
        var updated = manualUpdatedEvent.WaitOne(5000);
        Assert.IsTrue(updated);
    }

    [Test]
    public void TestInvite()
    {
        var testChannel = chat.CreatePublicConversation("test_invite_channel");
        var testUser = chat.CreateUser("test_invite_user");
        var returnedMembership = testChannel.Invite(testUser);
        Assert.True(returnedMembership.ChannelId == testChannel.Id && returnedMembership.UserId == testUser.Id);
    }

    [Test]
    public void TestInviteMultiple()
    {
        var testChannel = chat.CreatePublicConversation("invite_multiple_test_channel");
        var secondUser = chat.CreateUser("second_invite_user");
        var returnedMemberships = testChannel.InviteMultiple([
            user,
            secondUser
        ]);
        Assert.True(
            returnedMemberships.Count == 2 && 
            returnedMemberships.Any(x => x.UserId == secondUser.Id && x.ChannelId == testChannel.Id) &&
            returnedMemberships.Any(x => x.UserId == user.Id && x.ChannelId == testChannel.Id));
    }
}