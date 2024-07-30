// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubThreadChannel.h"
#include "PubnubMessage.h"
#include "PubnubChatSubsystem.h"
#include "FunctionLibraries/PubnubChatUtilities.h"


UPubnubThreadChannel* UPubnubThreadChannel::Create(Pubnub::ThreadChannel ThreadChannel)
{
	UPubnubThreadChannel* NewChannel = NewObject<UPubnubThreadChannel>();
	NewChannel->InternalChannel = new Pubnub::ThreadChannel(ThreadChannel);
	NewChannel->IsThreadChannel = true;
	return NewChannel;
}

UPubnubMessage* UPubnubThreadChannel::GetParentMessage()
{
	if(!IsInternalChannelValid()) {return nullptr;}
	return UPubnubMessage::Create(GetInternalThreadChannel()->parent_message());
}

FString UPubnubThreadChannel::GetParentChannelID()
{
	if(!IsInternalChannelValid()) {return "";}
	return UPubnubChatUtilities::PubnubStringToFString(GetInternalThreadChannel()->parent_channel_id());
}

Pubnub::ThreadChannel* UPubnubThreadChannel::GetInternalThreadChannel()
{
	return (Pubnub::ThreadChannel*)(InternalChannel);
}

bool UPubnubThreadChannel::IsInternalThreadChannelValid()
{
	if(GetInternalThreadChannel() == nullptr)
	{
		UE_LOG(PubnubLog, Error, TEXT("This PubnubChannel is invalid"));
		return false;
	}
	return true;
}