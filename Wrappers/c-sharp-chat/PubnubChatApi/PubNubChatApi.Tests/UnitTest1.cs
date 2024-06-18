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
            
            var mainUserId = "mainUser";
            var chat = new Chat("pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf",
                "sub-c-2b4db8f2-c025-4a76-9e23-326123298667", mainUserId);

            //await Task.Delay(3000);
        
            var user = chat.CreateUser(mainUserId);
            var channel = chat.CreatePublicConversation("MainChannel");
            channel.OnMessageReceived += message => Debug.WriteLine($"Received message: {message.MessageText}");
            channel.Join();
            channel.SendText("Hello World from Pubnub!");

            await Task.Delay(10000);
        }
        catch (Exception ex)
        {
            Debug.WriteLine("Caught exception!");
            Debug.WriteLine(ex.ToString());
        }

        Assert.Pass();
    }
}