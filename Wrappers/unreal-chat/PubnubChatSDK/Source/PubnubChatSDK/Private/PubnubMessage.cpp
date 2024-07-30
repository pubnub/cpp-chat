// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubMessage.h"
#include "PubnubThreadChannel.h"
#include "PubnubChatSubsystem.h"
#include "FunctionLibraries/PubnubChatUtilities.h"

UPubnubMessage* UPubnubMessage::Create(Pubnub::Message Message)
{
	UPubnubMessage* NewMessage = NewObject<UPubnubMessage>();
	NewMessage->InternalMessage = new Pubnub::Message(Message);
	return NewMessage;
}

FString UPubnubMessage::GetTimetoken()
{
	if(!IsInternalMessageValid()) {return "";}
	
	return UPubnubChatUtilities::PubnubStringToFString(InternalMessage->timetoken());
}

FPubnubChatMessageData UPubnubMessage::GetMessageData()
{
	if(!IsInternalMessageValid()) {return FPubnubChatMessageData();}
	
	return FPubnubChatMessageData(InternalMessage->message_data());
}

void UPubnubMessage::EditText(FString NewText)
{
	if(!IsInternalMessageValid()) {return;}

	InternalMessage->edit_text(UPubnubChatUtilities::FStringToPubnubString(NewText));
}

FString UPubnubMessage::Text()
{
	if(!IsInternalMessageValid()) {return "";}

	return UPubnubChatUtilities::PubnubStringToFString(InternalMessage->text());
}

UPubnubMessage* UPubnubMessage::DeleteMessage()
{
	if(!IsInternalMessageValid()) {return nullptr;}

	return Create(InternalMessage->delete_message());
}

bool UPubnubMessage::Deleted()
{
	if(!IsInternalMessageValid()) {return false;}

	return InternalMessage->deleted();
}

EPubnubChatMessageType UPubnubMessage::Type()
{
	if(!IsInternalMessageValid()) {return EPubnubChatMessageType::PCMT_TEXT;}

	return (EPubnubChatMessageType)(uint8)InternalMessage->type();
}

void UPubnubMessage::Pin()
{
	if(!IsInternalMessageValid()) {return;}

	InternalMessage->pin();
}

void UPubnubMessage::Unpin()
{
	if(!IsInternalMessageValid()) {return;}

	InternalMessage->unpin();
}

UPubnubMessage* UPubnubMessage::ToggleReaction(FString Reaction)
{
	if(!IsInternalMessageValid()) {return nullptr;}

	auto CppMessage = InternalMessage->toggle_reaction(UPubnubChatUtilities::FStringToPubnubString(Reaction));
	return Create(CppMessage);
}

TArray<FPubnubMessageAction> UPubnubMessage::Reactions()
{
	if(!IsInternalMessageValid()) {return {};}

	auto CppActions = InternalMessage->reactions();
	return UPubnubChatUtilities::CppMessageActionsToUnrealMessageActions(CppActions);
}

bool UPubnubMessage::HasUserReaction(FString Reaction)
{
	if(!IsInternalMessageValid()) {return false;}

	return InternalMessage->has_user_reaction(UPubnubChatUtilities::FStringToPubnubString(Reaction));
}

void UPubnubMessage::Forward(FString ChannelID)
{
	if(!IsInternalMessageValid()) {return;}

	InternalMessage->forward(UPubnubChatUtilities::FStringToPubnubString(ChannelID));
}

void UPubnubMessage::Report(FString Reason)
{
	if(!IsInternalMessageValid()) {return;}

	InternalMessage->report(UPubnubChatUtilities::FStringToPubnubString(Reason));
}

void UPubnubMessage::StreamUpdates(FOnPubnubMessageStreamUpdateReceived MessageUpdateCallback)
{
	if(!IsInternalMessageValid()) {return;}

	auto lambda = [MessageUpdateCallback](Pubnub::Message Message)
	{
		AsyncTask(ENamedThreads::GameThread, [MessageUpdateCallback, Message]()
		{
			UPubnubMessage* NewMessage = UPubnubMessage::Create(Message);
			MessageUpdateCallback.ExecuteIfBound(NewMessage);
		});

	};
	InternalMessage->stream_updates(lambda);
}

void UPubnubMessage::StreamUpdatesOn(TArray<UPubnubMessage*> Messages, FOnPubnubMessageStreamUpdateReceived MessageUpdateCallback)
{
	if(!IsInternalMessageValid()) {return;}

	auto lambda = [MessageUpdateCallback](Pubnub::Message Message)
	{
		AsyncTask(ENamedThreads::GameThread, [MessageUpdateCallback, Message]()
		{
			UPubnubMessage* NewMessage = UPubnubMessage::Create(Message);
			MessageUpdateCallback.ExecuteIfBound(NewMessage);
		});
	};
	
	auto CppMessages = UPubnubChatUtilities::UnrealMessagesToCppMessages(Messages);

	InternalMessage->stream_updates_on(CppMessages, lambda);
}

UPubnubThreadChannel* UPubnubMessage::CreateThread()
{
	auto CppThread = InternalMessage->create_thread();
	return UPubnubThreadChannel::Create(CppThread);
}

UPubnubThreadChannel* UPubnubMessage::GetThread()
{
	auto CppThread = InternalMessage->get_thread();
	return UPubnubThreadChannel::Create(CppThread);
}

bool UPubnubMessage::HasThread()
{
	return InternalMessage->has_thread();
}

void UPubnubMessage::RemoveThread()
{
	InternalMessage->remove_thread();
}

bool UPubnubMessage::IsInternalMessageValid()
{
	if(InternalMessage == nullptr)
	{
		UE_LOG(PubnubLog, Error, TEXT("This PubnubMessage is invalid"));
		return false;
	}
	return true;
}
