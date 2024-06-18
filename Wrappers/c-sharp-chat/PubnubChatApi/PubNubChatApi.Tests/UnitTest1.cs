using System.Diagnostics;
using Newtonsoft.Json;
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

            var user = chat.CreateUser("user123");
            chat.CreateUser("user321");
            var testowy = chat.CreateUser("user555");

            await Task.Delay(3000);

            var channel = chat.CreatePublicConversation("nowy_lepszy_kanal");

            channel.SetRestrictions("user321", true, true, "some reason");

            await Task.Delay(3000);

            var res = channel.GetUserRestriction("user321", 50, "99999999999999999", "00000000000000000");

            Debug.WriteLine(res.Reason);

            user.SetRestriction("nowy_lepszy_kanal", true, true, "other reason");

            await Task.Delay(3000);

            var res2 = user.GetChannelRestriction("nowy_lepszy_kanal", 50, "99999999999999999", "00000000000000000");
            Debug.WriteLine(res2.Reason);

            chat.SetRestriction("user555", "nowy_lepszy_kanal", true, true, "some other reason");

            await Task.Delay(3000);

            var res3 = testowy.GetChannelRestriction("nowy_lepszy_kanal", 50, "99999999999999999",
                "00000000000000000");

            Debug.WriteLine(res3.Reason);
        }
        catch (Exception ex)
        {
            Debug.WriteLine("Caught exception!");
            Debug.WriteLine(ex.ToString());
        }

        Assert.Pass();
    }
}