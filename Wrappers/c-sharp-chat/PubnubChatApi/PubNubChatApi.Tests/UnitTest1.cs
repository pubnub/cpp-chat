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
                "pub-c-79c582a2-d7a4-4ee7-9f28-7a6f1b7fa11c",
                "sub-c-ca0af928-f4f9-474c-b56e-d6be81bf8ed0",
                "huehue_hue_v13");
            chat.OnEvent += s =>
            {
                Debug.WriteLine($"EVENT(TEMP): {s}");
            };
            var oldChannelData = new ChatChannelData()
            {
                ChannelName = "test",
                ChannelDescription = "fuck",
                ChannelCustomDataJson = "{}",
                ChannelStatus = "1",
                ChannelType = "sure",
                ChannelUpdated = "true"
            };

            var user = chat.CreateUser("huehue_hue_v13");
            
            await Task.Delay(7000);
            
            var channel = chat.CreatePublicConversation("nowy_kanal_elo_v13", oldChannelData);
            channel.OnPresenceUpdate += list =>
            {
                foreach (var element in list)
                {
                    Debug.WriteLine($"PRESENCE: {element}");
                }
            };

            await Task.Delay(7000);
            
            channel.Join();

            await Task.Delay(7000);

            user.OnUserUpdated += updatedUser =>
            {
                Debug.WriteLine($"NEW NAME: {updatedUser.UserName}");
            };
            
            user.UpdateUser(new ChatUserData()
            {
                Username = "HAAAAAAAAAAAAAA"
            });
            
            await Task.Delay(7000);
            
            foreach (var who in chat.GetUsers(string.Empty, 50, "99999999999999999", "00000000000000000"))
            {
                Debug.WriteLine($"User (before delete): {who.Id}");
            }
            
            user.DeleteUser();
            
            await Task.Delay(7000);
            
            foreach (var who in chat.GetUsers(string.Empty, 50, "99999999999999999", "00000000000000000"))
            {
                Debug.WriteLine($"User (after delete): {who.Id}");
            }
            
            channel.Leave();
            
        }
        catch (Exception ex)
        {
            Debug.WriteLine("Caught exception!");
            Debug.WriteLine(ex.ToString());
        }

        Assert.Pass();
    }
}