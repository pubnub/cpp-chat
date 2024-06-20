namespace PubnubChatApi.Entities.Data
{
    /// <summary>
    /// Data struct for restriction.
    /// </summary>
    public struct Restriction
    {
        public bool Ban;
        public bool Mute;
        public string Reason;
    }
}