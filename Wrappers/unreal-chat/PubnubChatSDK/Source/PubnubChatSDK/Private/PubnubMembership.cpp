// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubMembership.h"
#include "PubnubChatSubsystem.h"
#include "PubnubChannel.h"
#include "PubnubMessage.h"
#include "PubnubUser.h"
#include "FunctionLibraries/PubnubChatUtilities.h"

UPubnubMembership* UPubnubMembership::Create(Pubnub::Membership Membership)
{
	UPubnubMembership* NewMembership = NewObject<UPubnubMembership>();
	NewMembership->InternalMembership = new Pubnub::Membership(Membership);
	Channel = UPubnubChannel::Create(Membership.channel);
	User = UPubnubUser::Create(Membership.user);
	return NewMembership;
}

FString UPubnubMembership::GetCustomData()
{
	if(!IsInternalMembershipValid()) {return "";}

	return UPubnubChatUtilities::PubnubStringToFString(InternalMembership->custom_data());
}

UPubnubMembership* UPubnubMembership::Update(FString CustomData)
{
	if(!IsInternalMembershipValid()) {return nullptr;}

	auto CppMembership = InternalMembership->update(UPubnubChatUtilities::FStringToPubnubString(CustomData));
	return Create(CppMembership);
}

void UPubnubMembership::StreamUpdates(FOnPubnubMembershipStreamUpdateReceived MembershipUpdateCallback)
{
	if(!IsInternalMembershipValid()) {return;}

	auto lambda = [MembershipUpdateCallback](Pubnub::Membership Membership)
	{
		AsyncTask(ENamedThreads::GameThread, [MembershipUpdateCallback, Membership]()
		{
			UPubnubMembership* NewMembership = UPubnubMembership::Create(Membership);
			MembershipUpdateCallback.ExecuteIfBound(NewMembership);
		});

	};
	InternalMembership->stream_updates(lambda);
}

void UPubnubMembership::StreamUpdatesOn(TArray<UPubnubMembership*> Memberships, FOnPubnubMembershipStreamUpdateReceived MembershipUpdateCallback)
{
	if(!IsInternalMembershipValid()) {return;}
	
	auto lambda = [MembershipUpdateCallback](Pubnub::Membership Membership)
	{
		AsyncTask(ENamedThreads::GameThread, [MembershipUpdateCallback, Membership]()
		{
			UPubnubMembership* NewMembership = UPubnubMembership::Create(Membership);
			MembershipUpdateCallback.ExecuteIfBound(NewMembership);
		});
	};
	
	auto CppMemberships = UPubnubChatUtilities::UnrealMembershipsToCppMemberships(Memberships);

	InternalMembership->stream_updates_on(CppMemberships, lambda);
}

FString UPubnubMembership::LastReadMessageTimetoken()
{
	if(!IsInternalMembershipValid()) {return "";}

	return UPubnubChatUtilities::PubnubStringToFString(InternalMembership->last_read_message_timetoken());
}

UPubnubMembership* UPubnubMembership::SetLastReadMessageTimetoken(FString Timetoken)
{
	if(!IsInternalMembershipValid()) {return nullptr;}

	auto CppMembership = InternalMembership->set_last_read_message_timetoken(UPubnubChatUtilities::FStringToPubnubString(Timetoken));

	return Create(CppMembership);
}

UPubnubMembership* UPubnubMembership::SetLastReadMessage(UPubnubMessage* Message)
{
	if(!IsInternalMembershipValid() || !Message) {return nullptr;}

	auto CppMembership = InternalMembership->set_last_read_message(*Message->GetInternalMessage());

	return Create(CppMembership);
}

int UPubnubMembership::GetUnreadMessageCount()
{
	if(!IsInternalMembershipValid()) {return 0;}

	return InternalMembership->get_unread_messages_count();
}

bool UPubnubMembership::IsInternalMembershipValid()
{
	if(InternalMembership == nullptr)
	{
		UE_LOG(PubnubLog, Error, TEXT("This PubnubMembership is invalid"));
		return false;
	}
	return true;
}
