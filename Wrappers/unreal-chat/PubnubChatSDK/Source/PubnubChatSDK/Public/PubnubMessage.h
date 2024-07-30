// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <pubnub_chat/message.hpp>
#include "PubnubChatStructLibrary.h"
#include "PubnubMessage.generated.h"

class UPubnubMessage;
class UPubnubThreadChannel;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubMessageStreamUpdateReceived, UPubnubMessage*, PubnubMessage);


/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBCHATSDK_API UPubnubMessage : public UObject
{
	GENERATED_BODY()
public:
	static UPubnubMessage* Create(Pubnub::Message Message);
	~UPubnubMessage(){delete InternalMessage;}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub Message")
	FString GetTimetoken();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub Message")
	FPubnubChatMessageData GetMessageData();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void EditText(FString NewText);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub Message")
	FString Text();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	UPubnubMessage* DeleteMessage ();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	bool Deleted();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	EPubnubChatMessageType Type();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void Pin();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void Unpin();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	UPubnubMessage* ToggleReaction(FString Reaction);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	TArray<FPubnubMessageAction> Reactions();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	bool HasUserReaction(FString Reaction);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void Forward(FString ChannelID);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void Report(FString Reason);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void StreamUpdates(FOnPubnubMessageStreamUpdateReceived MessageUpdateCallback);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message")
	void StreamUpdatesOn(TArray<UPubnubMessage*> Messages, FOnPubnubMessageStreamUpdateReceived MessageUpdateCallback);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message | Threads")
	UPubnubThreadChannel* CreateThread();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Message | Threads")
	UPubnubThreadChannel* GetThread();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message | Threads")
	bool HasThread();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Message | Threads")
	void RemoveThread();
	


	
	//Internal usage only
	Pubnub::Message* GetInternalMessage(){return InternalMessage;};

private:
	Pubnub::Message* InternalMessage;

	bool IsInternalMessageValid();
};
