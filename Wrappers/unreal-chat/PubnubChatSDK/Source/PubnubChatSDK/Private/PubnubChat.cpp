// Fill out your copyright notice in the Description page of Project Settings.

#include "PubnubChat.h"
#include "PubnubUser.h"
#include "PubnubMessage.h"
#include "PubnubThreadChannel.h"
#include "PubnubChatSubsystem.h"
#include "FunctionLibraries/PubnubChatUtilities.h"

DEFINE_LOG_CATEGORY(PubnubLog)


UPubnubChat* UPubnubChat::Create(Pubnub::Chat Chat)
{
	UPubnubChat* NewChat = NewObject<UPubnubChat>();
	NewChat->InternalChat = new Pubnub::Chat(Chat);
	return NewChat;
}

UPubnubChannel* UPubnubChat::CreatePublicConversation(FString ChannelID, FPubnubChatChannelData ChannelData)
{
	auto channel = InternalChat->create_public_conversation(UPubnubChatUtilities::FStringToPubnubString(ChannelID), ChannelData.GetCppChatChannelData());
	return UPubnubChannel::Create(channel);
}

FPubnubCreatedChannelWrapper UPubnubChat::CreateGroupConversation(TArray<UPubnubUser*> Users, FString ChannelID, FPubnubChatChannelData ChannelData, FString MembershipData)
{
	auto CppUsers = UPubnubChatUtilities::UnrealUsersToCppUsers(Users);
	auto CppWrapper = InternalChat->create_group_conversation(CppUsers, UPubnubChatUtilities::FStringToPubnubString(ChannelID), ChannelData.GetCppChatChannelData(), UPubnubChatUtilities::FStringToPubnubString(MembershipData));
	FPubnubCreatedChannelWrapper UEWrapper(CppWrapper);
	return UEWrapper;
}

FPubnubCreatedChannelWrapper UPubnubChat::CreateDirectConversation(UPubnubUser* User, FString ChannelID, FPubnubChatChannelData ChannelData, FString MembershipData)
{
	auto CppWrapper = InternalChat->create_direct_conversation(*User->GetInternalUser(), UPubnubChatUtilities::FStringToPubnubString(ChannelID), ChannelData.GetCppChatChannelData(), UPubnubChatUtilities::FStringToPubnubString(MembershipData));
	FPubnubCreatedChannelWrapper UEWrapper(CppWrapper);
	return UEWrapper;
}

UPubnubChannel* UPubnubChat::GetChannel(FString ChannelID)
{
	auto channel = InternalChat->get_channel(UPubnubChatUtilities::FStringToPubnubString(ChannelID));
	return UPubnubChannel::Create(channel);
}

TArray<UPubnubChannel*> UPubnubChat::GetChannels(FString Include, int Limit, FString Start, FString End)
{
	TArray<UPubnubChannel*> FinalChannels;
	auto CppChannels = InternalChat->get_channels(UPubnubChatUtilities::FStringToPubnubString(Include), Limit, UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End));
	FinalChannels = UPubnubChatUtilities::CppChannelsToUnrealChannels(CppChannels);

	return FinalChannels;
}

UPubnubChannel* UPubnubChat::UpdateChannel(FString ChannelID, FPubnubChatChannelData ChannelData)
{
	auto new_channel_data = ChannelData.GetCppChatChannelData();
	auto channel = InternalChat->update_channel(UPubnubChatUtilities::FStringToPubnubString(ChannelID), ChannelData.GetCppChatChannelData());
	return UPubnubChannel::Create(channel);
}

void UPubnubChat::DeleteChannel(FString ChannelID)
{
	InternalChat->delete_channel(UPubnubChatUtilities::FStringToPubnubString(ChannelID));
}

void UPubnubChat::PinMessageToChannel(UPubnubMessage* Message, UPubnubChannel* Channel)
{
	InternalChat->pin_message_to_channel(*Message->GetInternalMessage(), *Channel->GetInternalChannel());
}

void UPubnubChat::UnpinMessageFromChannel(UPubnubChannel* Channel)
{
	InternalChat->unpin_message_from_channel(*Channel->GetInternalChannel());
}

UPubnubUser* UPubnubChat::CreateUser(FString UserID, FPubnubChatUserData UserData)
{
	auto CppUser = InternalChat->create_user(UPubnubChatUtilities::FStringToPubnubString(UserID), UserData.GetCppChatUserData());
	return UPubnubUser::Create(CppUser);
}

UPubnubUser* UPubnubChat::GetUser(FString UserID)
{
	auto CppUser = InternalChat->get_user(UPubnubChatUtilities::FStringToPubnubString(UserID));
	return UPubnubUser::Create(CppUser);
}

TArray<UPubnubUser*> UPubnubChat::GetUsers(FString Include, int Limit, FString Start, FString End)
{
	TArray<UPubnubUser*> FinalUsers;
	auto CppUsers = InternalChat->get_users(UPubnubChatUtilities::FStringToPubnubString(Include), Limit, UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End));
	FinalUsers = UPubnubChatUtilities::CppUsersToUnrealUsers(CppUsers);
	return FinalUsers;
}

UPubnubUser* UPubnubChat::UpdateUser(FString UserID, FPubnubChatUserData UserData)
{
	auto CppUser = InternalChat->update_user(UPubnubChatUtilities::FStringToPubnubString(UserID), UserData.GetCppChatUserData());
	return UPubnubUser::Create(CppUser);
}

void UPubnubChat::DeleteUser(FString UserID)
{
	InternalChat->delete_user(UPubnubChatUtilities::FStringToPubnubString(UserID));
}

TArray<FString> UPubnubChat::WherePresent(FString UserID)
{
	TArray<FString> PresentChannels;

	auto ResponseChannels = InternalChat->where_present(UPubnubChatUtilities::FStringToPubnubString(UserID));
	PresentChannels = UPubnubChatUtilities::PubnubStringsToFStrings(ResponseChannels);
	
	return PresentChannels;
}

TArray<FString> UPubnubChat::WhoIsPresent(FString ChannelID)
{
	TArray<FString> PresentUsers;
	
	auto ResponseUsers = InternalChat->who_is_present(UPubnubChatUtilities::FStringToPubnubString(ChannelID));
	PresentUsers = UPubnubChatUtilities::PubnubStringsToFStrings(ResponseUsers);
	
	return PresentUsers;
}

bool UPubnubChat::IsPresent(FString UserID, FString ChannelID)
{
	return InternalChat->is_present(UPubnubChatUtilities::FStringToPubnubString(UserID), UPubnubChatUtilities::FStringToPubnubString(ChannelID));
}

void UPubnubChat::SetRestrictions(FString UserID, FString ChannelID, FPubnubRestriction Restrictions)
{
	InternalChat->set_restrictions(UPubnubChatUtilities::FStringToPubnubString(UserID), UPubnubChatUtilities::FStringToPubnubString(ChannelID), Restrictions.GetCppRestriction());
}

void UPubnubChat::EmitChatEvent(EPubnubChatEventType ChatEventType, FString ChannelID, FString Payload)
{
	InternalChat->emit_chat_event((Pubnub::pubnub_chat_event_type)(uint8)ChatEventType, UPubnubChatUtilities::FStringToPubnubString(ChannelID), UPubnubChatUtilities::FStringToPubnubString(Payload));
}

void UPubnubChat::ListenForEvents(FString ChannelID, EPubnubChatEventType ChatEventType, FOnPubnubEventReceived EventCallback)
{
	auto lambda = [EventCallback](Pubnub::String Event)
	{
		AsyncTask(ENamedThreads::GameThread, [EventCallback, Event]()
		{
			EventCallback.ExecuteIfBound(UPubnubChatUtilities::PubnubStringToFString(Event));
		});
	};

	InternalChat->listen_for_events(UPubnubChatUtilities::FStringToPubnubString(ChannelID), (Pubnub::pubnub_chat_event_type)(uint8)ChatEventType, lambda);
}

void UPubnubChat::ForwardMessage(UPubnubChannel* Channel, UPubnubMessage* Message)
{
	InternalChat->forward_message(*Message->GetInternalMessage(), *Channel->GetInternalChannel());
}

TArray<FPubnubUnreadMessageWrapper> UPubnubChat::GetUnreadMessagesCounts(int Limit, FString Start, FString End, FString Filter)
{
	auto MessageWrappers = InternalChat->get_unread_messages_counts(UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End), UPubnubChatUtilities::FStringToPubnubString(Filter), Limit);
	auto MessageWrappersStd = MessageWrappers.into_std_vector();
	TArray<FPubnubUnreadMessageWrapper> FinalWrappers;
	
	for(auto Wrapper : MessageWrappersStd)
	{
		FinalWrappers.Add(Wrapper);
	}
	
	return FinalWrappers;
}

FPubnubMarkMessagesAsReadWrapper UPubnubChat::MarkAllMessagesAsRead(FString Filter, FString Sort, int Limit, FPubnubPage Page)
{
	auto CppWrapper = InternalChat->mark_all_messages_as_read(UPubnubChatUtilities::FStringToPubnubString(Filter), UPubnubChatUtilities::FStringToPubnubString(Sort), Limit, Page.GetCppPage());
	FPubnubMarkMessagesAsReadWrapper UEWrapper(CppWrapper);
	return UEWrapper;
}

UPubnubThreadChannel* UPubnubChat::CreateThreadChannel(UPubnubMessage* Message)
{
	auto CppThread = InternalChat->create_thread_channel(*Message->GetInternalMessage());
	return UPubnubThreadChannel::Create(CppThread);
}

UPubnubThreadChannel* UPubnubChat::GetThreadChannel(UPubnubMessage* Message)
{
	auto CppThread = InternalChat->get_thread_channel(*Message->GetInternalMessage());
	return UPubnubThreadChannel::Create(CppThread);
}

void UPubnubChat::RemoveThreadChannel(UPubnubMessage* Message)
{
	InternalChat->remove_thread_channel(*Message->GetInternalMessage());
}

