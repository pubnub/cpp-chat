using PubNubChatAPI.Entities;
using PubnubChatApi.Enums;

namespace PubNubChatApi.Tests;

public class ChatTests
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
            "chats_tests_user");
        channel = chat.CreatePublicConversation("chat_tests_channel");
        user = chat.CreateUser("chats_tests_user");
        channel.Join();
    }

    [Test]
    public void TestCreateDirectConversation()
    {
        var convoUser = chat.CreateUser("direct_conversation_user");
        var directConversation =
            chat.CreateDirectConversation(convoUser, "direct_conversation_test");
        Assert.True(directConversation.createdChannel is { Id: "direct_conversation_test" });
        Assert.True(directConversation.hostMembership != null && directConversation.hostMembership.UserId == user.Id);
        Assert.True(directConversation.inviteeMembership != null &&
                    directConversation.inviteeMembership.UserId == convoUser.Id);
    }

    [Test]
    public void TestCreateGroupConversation()
    {
        var convoUser1 = chat.CreateUser("group_conversation_user_1");
        var convoUser2 = chat.CreateUser("group_conversation_user_2");
        var convoUser3 = chat.CreateUser("group_conversation_user_3");
        var groupConversation =
            chat.CreateGroupConversation([convoUser1, convoUser2, convoUser3], "group_conversation_test");
        Assert.True(groupConversation.createdChannel is { Id: "group_conversation_test" });
        Assert.True(groupConversation.hostMembership != null && groupConversation.hostMembership.UserId == user.Id);
        Assert.True(groupConversation.inviteeMemberships is { Count: 3 });
        Assert.True(groupConversation.inviteeMemberships.Any(x =>
            x.UserId == convoUser1.Id && x.ChannelId == "group_conversation_test"));
    }

    [Test]
    public void TestForwardMessage()
    {
        var messageForwardReceivedManualEvent = new ManualResetEvent(false);
        
        var forwardingChannel = chat.CreatePublicConversation("forwarding_channel");
        forwardingChannel.OnMessageReceived += message =>
        {
            Assert.True(message.MessageText == "message_to_forward");
            messageForwardReceivedManualEvent.Set();
        };
        forwardingChannel.Join();
        
        channel.Join();
        channel.OnMessageReceived += message =>
        {
            chat.ForwardMessage(message, forwardingChannel);
        };
        
        channel.SendText("message_to_forward");

        var forwarded = messageForwardReceivedManualEvent.WaitOne(6000);
        Assert.True(forwarded);
    }
    
    [Test]
    public void TestEmitEvent()
    {
        var reportManualEvent = new ManualResetEvent(false);
        chat.OnReportEvent += reportEvent =>
        {
            Assert.True(reportEvent.Payload == "{\"test\":\"some_nonsense\", \"type\": \"report\"}");
            reportManualEvent.Set();
        };
        channel.Join();
        chat.EmitEvent(PubnubChatEventType.Report, channel.Id, "{\"test\":\"some_nonsense\"}");

        var eventReceived = reportManualEvent.WaitOne(5000);
        Assert.True(eventReceived);
    }
}