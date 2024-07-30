// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <pubnub_chat/channel.hpp>
#include "UObject/NoExportTypes.h"
#include "PubnubChatStructLibrary.h"
#include "PubnubChannel.generated.h"

class UPubnubMessage;
class UPubnubChannel;
class UPubnubMembership;
class UPubnubUser;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubChannelMessageReceived, UPubnubMessage*, PubnubMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubChannelStreamUpdateReceived, UPubnubChannel*, PubnubChannel);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubChannelStreamPresenceReceived, const TArray<FString>&, PresentUsersIDs);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubChannelTypingReceived, const TArray<FString>&, TypingUsersIDs);

/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBCHATSDK_API UPubnubChannel : public UObject
{
	GENERATED_BODY()
	
public:
	static UPubnubChannel* Create(Pubnub::Channel Channel);
	virtual ~UPubnubChannel();
	

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub Channel")
	FString GetChannelID();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pubnub Channel")
	FPubnubChatChannelData GetChannelData();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	UPubnubChannel* Update(FPubnubChatChannelData ChannelData); 

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void Connect(FOnPubnubChannelMessageReceived MessageCallback);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void Disconnect();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void Join(FOnPubnubChannelMessageReceived MessageCallback, FString CustomData = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void Leave();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void DeleteChannel();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void SendText(FString Message, FString MetaData);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void StreamUpdates(FOnPubnubChannelStreamUpdateReceived ChannelUpdateCallback);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void StreamUpdatesOn(TArray<UPubnubChannel*> Channels, FOnPubnubChannelStreamUpdateReceived ChannelUpdateCallback);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void StreamPresence(FOnPubnubChannelStreamPresenceReceived PresenceCallback);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	TArray<FString> WhoIsPresent();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	bool IsPresent(FString UserID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void SetRestrictions(FString UserID, FPubnubRestriction Restrictions);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	FPubnubRestriction GetUserRestrictions(FString UserID, int Limit, FString Start, FString End);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	UPubnubMessage* GetMessage(FString Timetoken);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	TArray<UPubnubMessage*> GetHistory(int Limit, FString Start, FString End);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	TArray<UPubnubMembership*> GetMembers(int Limit, FString Start, FString End);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	UPubnubMembership* Invite(UPubnubUser* User);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	TArray<UPubnubMembership*> InviteMultiple(TArray<UPubnubUser*> Users);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void StartTyping();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void StopTyping();
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void GetTyping(FOnPubnubChannelTypingReceived TypingCallback);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	UPubnubChannel* PinMessage(UPubnubMessage* Message);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	UPubnubChannel* UnpinMessage();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	UPubnubMessage* GetPinnedMessage();

	UFUNCTION(BlueprintCallable, Category = "Pubnub Channel")
	void ForwardMessage(UPubnubMessage* Message);
	
	//Internal usage only
	Pubnub::Channel* GetInternalChannel(){return InternalChannel;};

protected:
	Pubnub::Channel* InternalChannel;

	bool IsInternalChannelValid();
	bool IsThreadChannel = false;
};
