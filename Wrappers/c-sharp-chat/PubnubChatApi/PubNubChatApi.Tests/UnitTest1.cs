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
                "huehue_hue_v6");
            var oldChannelData = new ChatChannelData()
            {
                ChannelName = "test",
                ChannelDescription = "fuck",
                ChannelCustomDataJson = "{}",
                ChannelStatus = "1",
                ChannelType = "sure",
                ChannelUpdated = "true"
            };

            var user = chat.CreateUser("huehue_hue_v6");
            
            await Task.Delay(7000);
            
            var channel = chat.CreatePublicConversation("nowy_kanal_elo_v6", oldChannelData);

            await Task.Delay(7000);
            
            channel.Join();

            await Task.Delay(7000);
            
            var members = channel.GetMemberships(50, "99999999999999999", "00000000000000000");
            foreach (var member in members)
            {
                Debug.WriteLine(member.Id);
            }

            foreach (var where in user.WherePresent())
            {
                Debug.WriteLine($"User Where: {where}");
            }
            foreach (var who in channel.WhoIsPresent())
            {
                Debug.WriteLine($"Channel Who: {who}");
            }
            foreach (var who in chat.WhoIsPresent(channel.Id))
            {
                Debug.WriteLine($"Chat Who: {who}");
            }
            Debug.WriteLine($"User Is: {user.IsPresentOn(channel.Id)}");
            Debug.WriteLine($"Channel Is: {channel.IsUserPresent(user.Id)}");
            Debug.WriteLine($"Chat Is: {chat.IsPresent(user.Id, channel.Id)}");

            /*await Task.Delay(5000);

            Debug.WriteLine("made channel");

            var firstTimeToken = "";
            var lastTimeToken = "";

            channel.Join();
            Debug.WriteLine("joined");
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

            await Task.Delay(5000);*/

            /*if(chat.TryGetChannel("DESTROYED_V2", out var testChannel))
            {
                testChannel.DeleteChannel();

                /*testChannel.OnChannelUpdate += updatedChannel =>
                {
                    Debug.WriteLine(updatedChannel.Name);
                };
                Debug.WriteLine($"AAAAAAAAAAAAAAA {testChannel.Id}");
                testChannel.UpdateChannel(new ChatChannelData()
                {
                    ChannelName = "EDITED AAAAAA",
                    ChannelDescription = "fuck 2 fuck 2 fuck 2 fuck",
                    ChannelCustomDataJson = "{}",
                    ChannelStatus = "1",
                    ChannelType = "sure",
                    ChannelUpdated = "true"
                });
                #1#

                await Task.Delay(10000);
            }*/

            /*channels = chat.GetChannels(string.Empty, 50, "99999999999999999", "00000000000000000");
            foreach (var fetchedChannel in channels)
            {
                Debug.WriteLine(fetchedChannel.Id);
            }*/

            /*await Task.Delay(2000);

            Debug.WriteLine("bef. first send");
            channel.SendText("one");
            Debug.WriteLine("aft first send");

            await Task.Delay(2000);

            Debug.WriteLine("bef. second send");
            channel.SendText("two");
            Debug.WriteLine("aft second send");

            await Task.Delay(2000);

            Debug.WriteLine("bef. third send");
            channel.SendText("three");
            Debug.WriteLine("aft third send");*/

            /*chat.CreateUser("gostek", new ChatUserData()
            {
                CustomDataJson = string.Empty,
                Email = "wololo@gmail.com",
                ExternalId = "chujwi",
                ProfileUrl = "www.google.com",
                Status = "Alive",
                Username = "xxx_Gostek_xxx"
            });
            Debug.WriteLine("Created gostek");

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

            await Task.Delay(1000);

            var users = chat.GetUsers(string.Empty, 50, "99999999999999999", "00000000000000000");
            Debug.WriteLine("after get users");
            foreach (var user in users)
            {
                Debug.WriteLine(user.Id);
            }

            await Task.Delay(1000);

            var history =
                channel.GetMessageHistory(lastTimeToken, firstTimeToken, 50);
            Debug.WriteLine("after history get");

            foreach (var message in history)
            {
                Debug.WriteLine(message.MessageText);
            }

            Debug.WriteLine("after history logged");

            var channels = chat.GetChannels(string.Empty, 50, "99999999999999999", "00000000000000000");

            Debug.WriteLine("after GetChannels");

            foreach (var channelOnList in channels)
            {
                Debug.WriteLine(channelOnList.Id);
            }

            Debug.WriteLine("after channels loggged");

            Debug.WriteLine("-------------------------");*/

            /*if(channel.TryGetMessage(history[0].TimeToken, out var fetchedMessage))
            {
                //TODO: waiting for c-side fix
                /*fetchedMessage.OnMessageUpdated += (mes) =>
                {
                    Debug.WriteLine($"I WAS UPDATEDDDDDD: {mes.MessageText}");
                };

                fetchedMessage.EditMessageText("NEW TEXTTTTT");#1#

                await Task.Delay(5000);

                Debug.WriteLine($"Updated text: {fetchedMessage.MessageText}");

                Debug.WriteLine($"deleted before: {fetchedMessage.IsDeleted}");
                fetchedMessage.DeleteMessage();
                await Task.Delay(5000);
                Debug.WriteLine($"deleted after: {fetchedMessage.IsDeleted}");
            }*/

            //await Task.Delay(60000);
        }
        catch (Exception ex)
        {
            Debug.WriteLine("caught exception!");
            Debug.WriteLine(ex.ToString());
        }

        Assert.Pass();
    }
}