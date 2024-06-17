using System.Diagnostics;
using System.Runtime.InteropServices;
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
                "heheh");
            var channel = chat.CreatePublicConversation("test", new ChatChannelData()
            {
                ChannelName = "test",
                ChannelDescription = "fuck",
                ChannelCustomDataJson = "{}",
                ChannelStatus = "1",
                ChannelType = "sure",
                ChannelUpdated = "true"
            });

            var firstTimeToken = "";
            var lastTimeToken = "";
            
            channel.Join();
            channel.OnMessageReceived += (message) =>
            {
                Debug.WriteLine($"RECEIVED: {message.MessageText}");
                if (message.MessageText == "one")
                {
                    firstTimeToken = message.TimeToken;
                }

                if (message.MessageText == "three")
                {
                    lastTimeToken = message.TimeToken;
                }
            };

            await Task.Delay(2000);

            channel.SendText("one");

            await Task.Delay(2000);

            channel.SendText("two");

            await Task.Delay(2000);

            channel.SendText("three");

            //TODO: C-Core error "Objects API transaction reported an error"
            chat.CreateUser("gostek", new ChatUserData()
            {
                CustomDataJson = string.Empty,
                Email = "wololo@gmail.com",
                ExternalId = "chujwi",
                ProfileUrl = "www.google.com",
                Status = "Alive",
                Username = "xxx_Gostek_xxx"
            });
            Debug.WriteLine("Created gostek");
            //chat.CreateUser("gostek");

            if (chat.TryGetUser("heheh", out var mainUser))
            {
                Debug.WriteLine(mainUser.Id);
                Debug.WriteLine(mainUser.UserName);
            }
            else
            {
                Debug.WriteLine("CO KURWA");
            }

            await Task.Delay(2000);
            
            if (chat.TryGetUser("gostek", out var gostekUser))
            {
                Debug.WriteLine(gostekUser.Id);
                Debug.WriteLine(gostekUser.UserName);
            }
            else
            {
                Debug.WriteLine("CO KURWA");
            }

            await Task.Delay(5000);
            
            var history = 
                channel.GetMessageHistory(lastTimeToken, firstTimeToken, 100);
            Debug.WriteLine("after history get");

            foreach (var message in history)
            {
                Debug.WriteLine(message.MessageText);
            }
            
            await Task.Delay(60000);
        }
        catch (Exception ex)
        {
            Debug.WriteLine(ex.ToString());
        }

        Assert.Pass();
    }
}