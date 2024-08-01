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
            "membership_tests_user_19");
        channel = chat.CreatePublicConversation("membership_tests_channel");
        user = chat.CreateUser("membership_tests_user_19");
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

    [Test]
    public async Task TestLastRead()
    {
        var testChannel = chat.CreatePublicConversation("last_read_test_channel_22");
        testChannel.Join();
        
        await Task.Delay(4000);
        
        var membership = user.GetMemberships(20, "99999999999999999", "00000000000000000")
            .FirstOrDefault(x => x.ChannelId == testChannel.Id);
        if (membership == null)
        {
            Assert.Fail();
            return;
        }

        var messageReceivedManual = new ManualResetEvent(false);
        
        testChannel.OnMessageReceived += async message =>
        {
            membership.SetLastReadMessage(message);
            Console.WriteLine("this is fine 1");

            await Task.Delay(5000);

            var lastTimeToken = membership.GetLastReadMessageTimeToken();
            Console.WriteLine("this is fine 2");
            
            /*await Task.Delay(5000);

            Assert.True(lastTimeToken == message.TimeToken);

            membership.SetLastReadMessageTimeToken("99999999999999999");

            await Task.Delay(3000);

            Assert.True(membership.GetLastReadMessageTimeToken() == "99999999999999999");*/
            messageReceivedManual.Set();
        };
        testChannel.SendText("some_message");

        var received = messageReceivedManual.WaitOne(7000);
        Assert.True(received);
    }

    [Test]
    public async Task TestUnreadMessagesCount()
    {
        if (chat.TryGetChannel("unread_count_test_channel", out var existingChannel))
        {
            chat.DeleteChannel(existingChannel.Id);
            await Task.Delay(6000);
        }

        var unreadChannel = chat.CreatePublicConversation("unread_count_test_channel");
        unreadChannel.Join();
        unreadChannel.SendText("one");
        unreadChannel.SendText("two");
        unreadChannel.SendText("three");

        await Task.Delay(4000);

        var membership = chat.GetUserMemberships(user.Id, 20, "99999999999999999", "00000000000000000")
            .FirstOrDefault(x => x.ChannelId == unreadChannel.Id);
        Assert.True(membership != null && membership.GetUnreadMessagesCount() == 3);
    }
}