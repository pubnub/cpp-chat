// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubChannel.h"
#include "PubnubMessage.h"
#include "PubnubMembership.h"
#include "PubnubChatSubsystem.h"
#include "PubnubUser.h"
#include "FunctionLibraries/PubnubChatUtilities.h"


UPubnubChannel* UPubnubChannel::Create(Pubnub::Channel Channel)
{
	UPubnubChannel* NewChannel = NewObject<UPubnubChannel>();
	NewChannel->InternalChannel = new Pubnub::Channel(Channel);
	return NewChannel;
}
UPubnubChannel::~UPubnubChannel()
{
	if(!IsThreadChannel)
	{
		delete InternalChannel;
	}
}

FString UPubnubChannel::GetChannelID()
{
	if(!IsInternalChannelValid()) {return "";}
	
	return UPubnubChatUtilities::PubnubStringToFString(InternalChannel->channel_id());
}

FPubnubChatChannelData UPubnubChannel::GetChannelData()
{
	if(!IsInternalChannelValid()) {return FPubnubChatChannelData();}

	return FPubnubChatChannelData(InternalChannel->channel_data());
}

UPubnubChannel* UPubnubChannel::Update(FPubnubChatChannelData ChannelData)
{
	if(!IsInternalChannelValid()) {return nullptr;}
	
	auto CppChannel = InternalChannel->update(ChannelData.GetCppChatChannelData());
	return UPubnubChannel::Create(CppChannel);
}

void UPubnubChannel::Connect(FOnPubnubChannelMessageReceived MessageCallback)
{
	if(!IsInternalChannelValid()) {return;}
	
	auto lambda = [MessageCallback](Pubnub::Message Message)
	{
		AsyncTask(ENamedThreads::GameThread, [MessageCallback, Message]()
		{
			UPubnubMessage* NewMessage = UPubnubMessage::Create(Message);
			MessageCallback.ExecuteIfBound(NewMessage);
		});
	};
	InternalChannel->connect(lambda);
}

void UPubnubChannel::Disconnect()
{
	if(!IsInternalChannelValid()) {return;}
	
	InternalChannel->disconnect();
}

void UPubnubChannel::Join(FOnPubnubChannelMessageReceived MessageCallback, FString CustomData)
{
	if(!IsInternalChannelValid()) {return;}
	
	auto lambda = [MessageCallback](Pubnub::Message Message)
	{
		AsyncTask(ENamedThreads::GameThread, [MessageCallback, Message]()
		{
			UPubnubMessage* NewMessage = UPubnubMessage::Create(Message);
			MessageCallback.ExecuteIfBound(NewMessage);
		});

	};
	InternalChannel->join(lambda, UPubnubChatUtilities::FStringToPubnubString(CustomData));
}

void UPubnubChannel::Leave()
{
	if(!IsInternalChannelValid()) {return;}
	
	InternalChannel->leave();
}

void UPubnubChannel::DeleteChannel()
{
	if(!IsInternalChannelValid()) {return;}
	
	InternalChannel->delete_channel();
}

void UPubnubChannel::SendText(FString Message, FString MetaData)
{
	if(!IsInternalChannelValid()) {return;}

	InternalChannel->send_text(UPubnubChatUtilities::FStringToPubnubString(Message), Pubnub::pubnub_chat_message_type::PCMT_TEXT, UPubnubChatUtilities::FStringToPubnubString(MetaData));
}

void UPubnubChannel::StreamUpdates(FOnPubnubChannelStreamUpdateReceived ChannelUpdateCallback)
{
	if(!IsInternalChannelValid()) {return;}
	
	auto lambda = [ChannelUpdateCallback](Pubnub::Channel Channel)
	{
		AsyncTask(ENamedThreads::GameThread, [ChannelUpdateCallback, Channel]()
		{
			UPubnubChannel* NewChannel = UPubnubChannel::Create(Channel);
			ChannelUpdateCallback.ExecuteIfBound(NewChannel);
		});

	};
	InternalChannel->stream_updates(lambda);
}

void UPubnubChannel::StreamUpdatesOn(TArray<UPubnubChannel*> Channels, FOnPubnubChannelStreamUpdateReceived ChannelUpdateCallback)
{
	if(!IsInternalChannelValid()) {return;}
	
	auto lambda = [ChannelUpdateCallback](Pubnub::Channel Channel)
	{
		AsyncTask(ENamedThreads::GameThread, [ChannelUpdateCallback, Channel]()
		{
			UPubnubChannel* NewChannel = UPubnubChannel::Create(Channel);
			ChannelUpdateCallback.ExecuteIfBound(NewChannel);
		});
	};
	
	auto CppChannels = UPubnubChatUtilities::UnrealChannelsToCppChannels(Channels);

	InternalChannel->stream_updates_on(Pubnub::Vector<Pubnub::Channel>(std::move(CppChannels)), lambda);
}

void UPubnubChannel::StreamPresence(FOnPubnubChannelStreamPresenceReceived PresenceCallback)
{
	if(!IsInternalChannelValid()) {return;}

	Pubnub::Vector<Pubnub::String> CppPresentUsers2;
	CppPresentUsers2.into_std_vector();
	
	auto lambda = [PresenceCallback](Pubnub::Vector<Pubnub::String> CppPresentUsers)
	{
		auto StdCppPresentUsers = CppPresentUsers.into_std_vector();
		AsyncTask(ENamedThreads::GameThread, [PresenceCallback, StdCppPresentUsers]()
		{
			TArray<FString> FinalPresentUsers;
			for(auto &CppUser : StdCppPresentUsers)
			{
				FinalPresentUsers.Add(UPubnubChatUtilities::PubnubStringToFString(CppUser));
			}
			PresenceCallback.ExecuteIfBound(FinalPresentUsers);
		});

	};
	InternalChannel->stream_presence(lambda);
}

TArray<FString> UPubnubChannel::WhoIsPresent()
{
	TArray<FString> PresentUsers;

	if(!IsInternalChannelValid()) {return PresentUsers;}

	auto ResponseUsers = InternalChannel->who_is_present();
	PresentUsers = UPubnubChatUtilities::PubnubStringsToFStrings(ResponseUsers);
	
	return PresentUsers;
}

bool UPubnubChannel::IsPresent(FString UserID)
{
	if(!IsInternalChannelValid()) {return false;}
	
	return InternalChannel->is_present(UPubnubChatUtilities::FStringToPubnubString(UserID));
}

void UPubnubChannel::SetRestrictions(FString UserID, FPubnubRestriction Restrictions)
{
	if(!IsInternalChannelValid()) {return;}

	InternalChannel->set_restrictions(UPubnubChatUtilities::FStringToPubnubString(UserID), Restrictions.GetCppRestriction());
}

FPubnubRestriction UPubnubChannel::GetUserRestrictions(FString UserID, int Limit, FString Start, FString End)
{
	if(!IsInternalChannelValid()) {return FPubnubRestriction();}

	return FPubnubRestriction(InternalChannel->get_user_restrictions(UPubnubChatUtilities::FStringToPubnubString(UserID), UPubnubChatUtilities::FStringToPubnubString(GetChannelID()), Limit, UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End)));
}

UPubnubMessage* UPubnubChannel::GetMessage(FString Timetoken)
{
	if(!IsInternalChannelValid()) {return nullptr;}
	
	auto CppMessage = InternalChannel->get_message(UPubnubChatUtilities::FStringToPubnubString(Timetoken));
	return UPubnubMessage::Create(CppMessage);
}

TArray<UPubnubMessage*> UPubnubChannel::GetHistory(int Limit, FString Start, FString End)
{
	TArray<UPubnubMessage*> FinalMessages;
	
	if(!IsInternalChannelValid()) {return FinalMessages;}

	auto CppHistoryMessages = InternalChannel->get_history(UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End), Limit).into_std_vector();

	//auto CppHistoryMessages =  CppHistoryMessages_vec.into_std_vector();
	//FinalMessages = UPubnubChatUtilities::CppMessagesToUnrealMessages(CppHistoryMessages);

	for(auto CppMessage : CppHistoryMessages)
	{
		FinalMessages.Add(UPubnubMessage::Create(CppMessage));
	}

	return FinalMessages;
}

TArray<UPubnubMembership*> UPubnubChannel::GetMembers(int Limit, FString Start, FString End)
{
	TArray<UPubnubMembership*> FinalMemberships;
	
	if(!IsInternalChannelValid()) {return FinalMemberships;}

	auto CppMemberships = InternalChannel->get_members(Limit, UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End));

	FinalMemberships = UPubnubChatUtilities::CppMembershipsToUnrealMemberships(CppMemberships);

	return FinalMemberships;
}

UPubnubMembership* UPubnubChannel::Invite(UPubnubUser* User)
{
	if(!IsInternalChannelValid()) {return nullptr;}

	auto CppMembership = InternalChannel->invite(*User->GetInternalUser());
	return UPubnubMembership::Create(CppMembership);
}

TArray<UPubnubMembership*> UPubnubChannel::InviteMultiple(TArray<UPubnubUser*> Users)
{
	TArray<UPubnubMembership*> FinalMemberships;
	if(!IsInternalChannelValid()) {return FinalMemberships;}

	auto CppUsers = UPubnubChatUtilities::UnrealUsersToCppUsers(Users);
	auto CppMemberships = InternalChannel->invite_multiple(std::move(CppUsers));
	FinalMemberships = UPubnubChatUtilities::CppMembershipsToUnrealMemberships(CppMemberships);
	
	return FinalMemberships;
}

void UPubnubChannel::StartTyping()
{
	if(!IsInternalChannelValid()) {return;}

	InternalChannel->start_typing();
}

void UPubnubChannel::StopTyping()
{
	if(!IsInternalChannelValid()) {return;}

	InternalChannel->stop_typing();
}

void UPubnubChannel::GetTyping(FOnPubnubChannelTypingReceived TypingCallback)
{
	if(!IsInternalChannelValid()) {return;}
	
	auto lambda = [TypingCallback](std::vector<Pubnub::String> CppTypingUsers)
	{
		AsyncTask(ENamedThreads::GameThread, [TypingCallback, CppTypingUsers]()
		{
			TArray<FString> FinalTypingUsers;
			for(auto &CppUser : CppTypingUsers)
			{
				FinalTypingUsers.Add(UPubnubChatUtilities::PubnubStringToFString(CppUser));
			}
			TypingCallback.ExecuteIfBound(FinalTypingUsers);
		});
	};
}

UPubnubChannel* UPubnubChannel::PinMessage(UPubnubMessage* Message)
{
	if(!IsInternalChannelValid()) {return nullptr;}

	return Create(InternalChannel->pin_message(*Message->GetInternalMessage()));
}

UPubnubChannel* UPubnubChannel::UnpinMessage()
{
	if(!IsInternalChannelValid()) {return nullptr;}

	return Create(InternalChannel->unpin_message());
}

UPubnubMessage* UPubnubChannel::GetPinnedMessage()
{
	if(!IsInternalChannelValid()) {return nullptr;}

	return UPubnubMessage::Create(InternalChannel->get_pinned_message());
}

void UPubnubChannel::ForwardMessage(UPubnubMessage* Message)
{
	if(!IsInternalChannelValid()) {return;}

	InternalChannel->forward_message(*Message->GetInternalMessage());
}

bool UPubnubChannel::IsInternalChannelValid()
{
	if(InternalChannel == nullptr)
	{
		UE_LOG(PubnubLog, Error, TEXT("This PubnubChannel is invalid"));
		return false;
	}
	return true;
}
