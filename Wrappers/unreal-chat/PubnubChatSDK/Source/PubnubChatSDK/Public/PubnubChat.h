// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PubnubChannel.h"
#include "PubnubMembership.h"
#include "PubnubChatStructLibrary.h"
#include "PubnubChat.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPubnubEventReceived, FString, Event);

class UPubnubChannel;
class UPubnubUser;
class UPubnubMessage;
class UPubnubMembership;
class UPubnubThreadChannel;

USTRUCT(BlueprintType)
struct FPubnubUnreadMessageWrapper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) UPubnubChannel* Channel;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) UPubnubMembership* Membership;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) int Count;

	FPubnubUnreadMessageWrapper() = default;
	FPubnubUnreadMessageWrapper(Pubnub::UnreadMessageWrapper MessageWrapper) :
	Count(MessageWrapper.count)
	{
		Channel = UPubnubChannel::Create(MessageWrapper.channel);
		Membership = UPubnubMembership::Create(MessageWrapper.membership);
		
	};
};

USTRUCT(BlueprintType)
struct FPubnubMarkMessagesAsReadWrapper
{
	GENERATED_BODY();
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) FPubnubPage Page;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) int Total;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) int Status;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) TArray<UPubnubMembership*> Memberships;

	FPubnubMarkMessagesAsReadWrapper() = default;
	FPubnubMarkMessagesAsReadWrapper(Pubnub::MarkMessagesAsReadWrapper& Wrapper) :
	Page(Wrapper.page),
	Total(Wrapper.total),
	Status(Wrapper.status)
	{
		auto CppMemberships = Wrapper.memberships.into_std_vector();
		for(auto Membership : CppMemberships)
		{
			Memberships.Add(UPubnubMembership::Create(Membership));
		}
	};
};

USTRUCT(BlueprintType)
struct FPubnubCreatedChannelWrapper
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) UPubnubChannel* CreatedChannel;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) UPubnubMembership* HostMembership;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere) TArray<UPubnubMembership*> InviteesMemberships;

	FPubnubCreatedChannelWrapper() = default;
	FPubnubCreatedChannelWrapper(Pubnub::CreatedChannelWrapper& Wrapper) :
	CreatedChannel(UPubnubChannel::Create(Wrapper.created_channel)),
	HostMembership(UPubnubMembership::Create(Wrapper.host_membership))
	{
		auto CppMemberships = Wrapper.invitees_memberships.into_std_vector();
		for(auto Membership : CppMemberships)
		{
			InviteesMemberships.Add(UPubnubMembership::Create(Membership));
		}
	}
};

/**
 * 
 */
UCLASS(BlueprintType)
class PUBNUBCHATSDK_API UPubnubChat : public UObject
{
	GENERATED_BODY()
	
public:
	static UPubnubChat* Create(Pubnub::Chat Chat);
	~UPubnubChat(){delete InternalChat;}

	
		/* CHANNELS */
	
	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	UPubnubChannel* CreatePublicConversation(FString ChannelID, FPubnubChatChannelData ChannelData);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	FPubnubCreatedChannelWrapper CreateGroupConversation(TArray<UPubnubUser*> Users, FString ChannelID, FPubnubChatChannelData ChannelData, FString MembershipData = "");

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	FPubnubCreatedChannelWrapper CreateDirectConversation(UPubnubUser* User, FString ChannelID, FPubnubChatChannelData ChannelData, FString MembershipData = "");

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	UPubnubChannel* GetChannel(FString ChannelID);
	
	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	TArray<UPubnubChannel*> GetChannels(FString Include,  int Limit, FString Start, FString End);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	UPubnubChannel* UpdateChannel(FString ChannelID, FPubnubChatChannelData ChannelData);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	void DeleteChannel(FString ChannelID);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	void PinMessageToChannel(UPubnubMessage* Message, UPubnubChannel* Channel);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|Channel")
	void UnpinMessageFromChannel(UPubnubChannel* Channel);

	
	/* USERS */

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|User")
	UPubnubUser* CreateUser(FString UserID, FPubnubChatUserData UserData);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|User")
	UPubnubUser* GetUser(FString UserID);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|User")
	TArray<UPubnubUser*> GetUsers(FString Include,  int Limit, FString Start, FString End);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|User")
	UPubnubUser* UpdateUser(FString UserID, FPubnubChatUserData UserData);

	UFUNCTION(BlueprintCallable, Category="Pubnub Chat|User")
	void DeleteUser(FString UserID);


	/* PRESENCE */

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Presence")
	TArray<FString> WherePresent(FString UserID);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Presence")
	TArray<FString> WhoIsPresent(FString ChannelID);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Presence")
	bool IsPresent(FString UserID, FString ChannelID);	

	
	/* MODERATION */

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Moderation")
	void SetRestrictions(FString UserID, FString ChannelID, FPubnubRestriction Restrictions);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Moderation")
	void EmitChatEvent(EPubnubChatEventType ChatEventType, FString ChannelID, FString Payload);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Moderation")
	void ListenForEvents(FString ChannelID, EPubnubChatEventType ChatEventType, FOnPubnubEventReceived EventCallback);

	/* MESSAGES */

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Messages")
	void ForwardMessage(UPubnubChannel* Channel, UPubnubMessage* Message);

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Messages")
	TArray<FPubnubUnreadMessageWrapper> GetUnreadMessagesCounts(int Limit, FString Start, FString End, FString Filter = "");

	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Messages")
	FPubnubMarkMessagesAsReadWrapper MarkAllMessagesAsRead(FString Filter = "", FString Sort = "", int Limit = 0, FPubnubPage Page = FPubnubPage());

	
    /* THREADS */
	
    UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Messages")
    UPubnubThreadChannel* CreateThreadChannel(UPubnubMessage* Message);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Messages")
	UPubnubThreadChannel* GetThreadChannel(UPubnubMessage* Message);
	
	UFUNCTION(BlueprintCallable, Category = "Pubnub Chat|Messages")
	void RemoveThreadChannel(UPubnubMessage* Message);
    
    
private:
	Pubnub::Chat* InternalChat;

};
