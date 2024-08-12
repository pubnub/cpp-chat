using System.Diagnostics;
using PubNubChatAPI.Entities;
using PubnubChatApi.Entities.Data;

namespace PubNubChatApi.Tests;

public class UserTests
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
            "user_tests_user");
        channel = chat.CreatePublicConversation("user_tests_channel");
        user = chat.CreateUser("user_tests_user");
        channel.Join();
    }

    [Test]
    public async Task TestUserUpdate()
    {
        var updatedReset = new ManualResetEvent(false);
        var newRandomUserName = Guid.NewGuid().ToString();
        user.OnUserUpdated += updatedUser =>
        {
            Assert.True(updatedUser.UserName == newRandomUserName);
            updatedReset.Set();
        };
        user.Update(new ChatUserData()
        {
            Username = newRandomUserName
        });
        var updated = updatedReset.WaitOne(8000);
        Assert.True(updated);
    }
}