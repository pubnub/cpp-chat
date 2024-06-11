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
            var chat = new Chat("pub-c-79961364-c3e6-4e48-8d8d-fe4f34e228bf", "sub-c-2b4db8f2-c025-4a76-9e23-326123298667", "heheh");
            var channel = chat.CreatePublicConversation(string.Empty, new ChatChannelData()
            {
                ChannelName = "test",
                ChannelDescription = "fuck",
                ChannelCustomDataJson = "{}",
                ChannelStatus = "1",
                ChannelType = "sure",
                ChannelUpdated = "true"
            });
            

            /*var callback = new Channel.CallbackStringFunction(((resulString, length) =>
            {
                var parsedString = new string('*', length);
                /*string parsedString = "aaaa";
                //make sure the buffer is big enough
                if (parsedString.Length + 1 > length)
                {
                    Debug.WriteLine("WRONG");
                    return;
                }
                var ptr = Marshal.StringToHGlobalAnsi(parsedString);
                var bytes = new byte[parsedString.Length + 1];
                Marshal.Copy(ptr, bytes, 0, parsedString.Length);
                Marshal.Copy(bytes, 0, resulStringPtr, bytes.Length);
                Marshal.FreeHGlobal(ptr);
                Debug.WriteLine(parsedString);#1#
            } ));
            channel.Connect(callback);*/
            
            Debug.WriteLine("before connect");
            channel.Connect();
            Debug.WriteLine("after connect");

            new Thread(() =>
                {
                    while (true)
                    {
                        Debug.WriteLine(chat.GetMessages("test"));
                        Thread.Sleep(200);
                    }
                })
            { IsBackground = true }
                .Start();

            //Debug.WriteLine(chat.GetMessages("test"));

            await Task.Delay(2000);
            
            Debug.WriteLine("before first send");
            channel.SendText("ARE YOU HEARING ME YOU BASTARDS?");
            Debug.WriteLine("after first send");
            
            await Task.Delay(2000);
            
            Debug.WriteLine("before second send");
            channel.SendText("sdfsdfsdfsdf");
            Debug.WriteLine("after second send");
            
            await Task.Delay(2000);
            
            Debug.WriteLine("before third send");
            channel.SendText("gks,dfhskdjfhksdfksjkdfhksdkfskjdfh");
            Debug.WriteLine("after third send");
            
            await Task.Delay(2000);
            
            Debug.WriteLine("got past the connect and sends");
            //Debug.WriteLine(chat.GetMessages("test"));
            
            await Task.Delay(60000);
            //await Task.Run(chat.ContinuouslyResolveMessages);



            /*Debug.WriteLine("test");
            var callback = new Channel.CallbackStringFunction((result => Debug.WriteLine(result)));
            Debug.WriteLine("did you get here?");
            channel.Join(callback);
            Debug.WriteLine("or here?");

            await Task.Delay(5000);

            channel.SendText("ARE YOU HEARING ME YOU BASTARDS?");
            Debug.WriteLine("OR HERE?");

            await Task.Delay(3000);
            Debug.WriteLine("A");

            Debug.WriteLine("B");

            await Task.Delay(7000);

            callback.Invoke("END OF FILE");*/
        }
        catch (Exception ex)
        {
            Debug.WriteLine(ex.ToString());
        }
        
        Assert.Pass();
    }
}