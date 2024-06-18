using System.Diagnostics;
using PubNubChatAPI.Entities;

namespace PubNubChatApi.Tests;

public class Tests
{
    [SetUp]
    public void Setup()
    {
    }

    [Test]
    public async Task Test1()
    {
        try
        {
            var chat = new Chat(
                "pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf",
                "sub-c-2b4db8f2-c025-4a76-9e23-326123298667",
                "Sebastian1234");
            var user = chat.CreateUser("Sebastian123");
            chat.CreatePublicConversation("jakistamkolega");

            await Task.Delay(3000);
            
            chat.StartListeningForReportEvents();
            chat.OnReportEvent += eventData =>
            {
                Console.WriteLine($"REPORT EVENT: {eventData}");
            };
            
            chat.StartListeningForUserModerationEvents("Sebastian123");
            chat.OnModerationEvent += eventData =>
            {
                Console.WriteLine($"MODERATION EVENT: {eventData}");
            };
            
            await Task.Delay(3000);

            chat.SetRestriction("Sebastian123", "jakistamkolega", new Restriction()
            {
                Ban = true,
                Mute = true,
                Reason = "cause fuck you"
            });

            await Task.Delay(3000);

            user.ReportUser("BUDDY FUCK YOU");
            
            await Task.Delay(9000);
        }
        catch (Exception ex)
        {
            Debug.WriteLine("Caught exception!");
            Debug.WriteLine(ex.ToString());
        }

        Assert.Pass();
    }
}