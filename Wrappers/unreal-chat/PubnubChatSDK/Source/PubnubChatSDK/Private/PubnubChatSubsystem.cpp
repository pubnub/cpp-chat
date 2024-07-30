// Copyright Epic Games, Inc. All Rights Reserved.

#include "PubnubChatSubsystem.h"

#include "ChatSDK.h"
#include "PubnubChat.h"
#include "FunctionLibraries/PubnubChatUtilities.h"


void UPubnubChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPubnubChatSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UPubnubChat* UPubnubChatSubsystem::InitChat(FString PublishKey, FString SubscribeKey, FString UserID, FPubnubChatConfig Config)
{
	if(Chat != nullptr)
	{
		UE_LOG(PubnubLog, Warning, TEXT("Chat already exists. (Only one chat object can be created). Returning existing Char"));
		return Chat;
	}
	
	Pubnub::ChatConfig CppConfig;
	CppConfig.publish_key = UPubnubChatUtilities::FStringToPubnubString(PublishKey);
	CppConfig.subscribe_key = UPubnubChatUtilities::FStringToPubnubString(SubscribeKey);
	CppConfig.user_id = UPubnubChatUtilities::FStringToPubnubString(UserID);
	
	Chat = UPubnubChat::Create(Pubnub::Chat(CppConfig));
	return Chat;
}

UPubnubChat* UPubnubChatSubsystem::GetChat()
{
	if(Chat == nullptr)
	{
		UE_LOG(PubnubLog, Warning, TEXT("Chat doesn't exist. Call 'Create Chat' instead"));
		return nullptr;
	}

	return Chat;
}
