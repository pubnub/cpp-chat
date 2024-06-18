using System.Threading.Tasks;
using PubNubChatAPI.Entities;
using UnityEngine;

/// <summary>
/// Contains a simple example of creating and using the Pubnub Chat API for Unity
/// </summary>
public class PubnubChatSample : MonoBehaviour
{
    [SerializeField] private string pubnubPubKey;
    [SerializeField] private string pubnubSubKey;
    
    private async void Start()
    {
        if (string.IsNullOrEmpty(pubnubSubKey) || string.IsNullOrEmpty(pubnubPubKey))
        {
            Debug.LogError("Provide Pubnub keys to run Chat example!");
            return;
        }
        
        //Define the main user ID
        var mainUserId = "mainUser";
        //Initialize Chat instance with Pubnub keys + user ID
        var chat = new Chat(pubnubPubKey, pubnubSubKey, mainUserId);
        //Create main user handle
        var user = chat.CreateUser(mainUserId);
        
        //Create a new channel
        var channel = chat.CreatePublicConversation("MainChannel");
        //Define reaction on receiving new messages
        channel.OnMessageReceived += message => Debug.Log($"Received message: {message.MessageText}");
        //Join channel
        channel.Join();
        //Send test message
        channel.SendText("Hello World from Pubnub!");
        
        //React on user data being updated
        user.OnUserUpdated += updatedUser =>
            Debug.Log($"{updatedUser.Id} has been updated! Their name is now {updatedUser.UserName}");
        //Update our user data
        user.UpdateUser(new ChatUserData()
        {
            Username = "FancyUserName"
        });
        
        //Send a few more messages
        channel.SendText("Hi!");
        channel.SendText("Hello!");
        channel.SendText("Anyone there?");
        channel.SendText("Me!");

        //Wait a moment to wait for them to be processed
        await Task.Delay(3000);
        
        //Fetch message history (from all time)
        foreach (var historyMessage in channel.GetMessageHistory("99999999999999999", "00000000000000000", 50))
        {
            Debug.Log($"Message from history with timetoken {historyMessage.TimeToken}: {historyMessage.MessageText}");
        }
    }
}