using PubNubChatAPI.Entities;
using PubnubChatApi.Entities.Data;

namespace PubNubChatApi.Tests;

public class RestrictionsTests
{
    private Chat chat;

    [SetUp]
    public void Setup()
    {
        chat = new Chat(
            PubnubTestsParameters.PublishKey,
            PubnubTestsParameters.SubscribeKey,
            "restrictions_tests_user");
    }

    [Test]
    public async Task TestSetRestrictions()
    {
        var user = chat.CreateUser("user123");
        var channel = chat.CreatePublicConversation("new_channel");

        await Task.Delay(2000);

        var restriction = new Restriction()
        {
            Ban = true,
            Mute = true,
            Reason = "Some Reason"
        };
        channel.SetRestrictions(user.Id, restriction);

        await Task.Delay(3000);

        var fetchedRestriction =
            channel.GetUserRestriction(user);

        Assert.True(restriction.Ban == fetchedRestriction.Ban && restriction.Mute == fetchedRestriction.Mute &&
                    restriction.Reason == fetchedRestriction.Reason);

        var restrictionFromUser = user.GetChannelRestriction(channel);
        
        Assert.True(restriction.Ban == restrictionFromUser.Ban && restriction.Mute == restrictionFromUser.Mute &&
                    restriction.Reason == restrictionFromUser.Reason);
    }
}