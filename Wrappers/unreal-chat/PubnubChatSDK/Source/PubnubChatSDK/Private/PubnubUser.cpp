// Fill out your copyright notice in the Description page of Project Settings.


#include "PubnubUser.h"
#include "PubnubChatSubsystem.h"
#include "FunctionLibraries/PubnubChatUtilities.h"

UPubnubUser* UPubnubUser::Create(Pubnub::User User)
{
	UPubnubUser* NewUser = NewObject<UPubnubUser>();
	NewUser->InternalUser = new Pubnub::User(User);
	return NewUser;
}

FString UPubnubUser::GetUserID()
{
	if(!IsInternalUserValid()) {return "";}

	return UPubnubChatUtilities::PubnubStringToFString(InternalUser->user_id());
}

FPubnubChatUserData UPubnubUser::GetUserData()
{
	if(!IsInternalUserValid()) {return FPubnubChatUserData();}

	return FPubnubChatUserData(InternalUser->user_data());
}

UPubnubUser* UPubnubUser::Update(FPubnubChatUserData UserData)
{
	if(!IsInternalUserValid()) {return nullptr;}

	auto CppUser = InternalUser->update(UserData.GetCppChatUserData());
	return UPubnubUser::Create(CppUser);
}

void UPubnubUser::DeleteUser()
{
	if(!IsInternalUserValid()) {return;}

	InternalUser->delete_user();
}

TArray<FString> UPubnubUser::WherePresent()
{
	TArray<FString> PresentChannels;
	
	if(!IsInternalUserValid()) {FPubnubChatUserData();}

	auto ResponseChannels = InternalUser->where_present();
	PresentChannels = UPubnubChatUtilities::PubnubStringsToFStrings(ResponseChannels);
	
	return PresentChannels;
}

bool UPubnubUser::IsPresentOn(FString ChannelID)
{
	if(!IsInternalUserValid()) {return false;}
	
	return InternalUser->is_present_on(UPubnubChatUtilities::FStringToPubnubString(ChannelID));
}

void UPubnubUser::SetRestrictions(FString ChannelID, FPubnubRestriction Restrictions)
{
	if(!IsInternalUserValid()) {return;}

	InternalUser->set_restrictions(UPubnubChatUtilities::FStringToPubnubString(ChannelID), Restrictions.GetCppRestriction());
}

FPubnubRestriction UPubnubUser::GetChannelRestriction(FString ChannelID, int Limit, FString Start, FString End)
{
	if(!IsInternalUserValid()) {return FPubnubRestriction();}

	return FPubnubRestriction(InternalUser->get_channel_restrictions(UPubnubChatUtilities::FStringToPubnubString(GetUserID()), UPubnubChatUtilities::FStringToPubnubString(ChannelID),
		Limit, UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End)));
}

void UPubnubUser::Report(FString Reason)
{
	if(!IsInternalUserValid()) {return;}

	InternalUser->report(UPubnubChatUtilities::FStringToPubnubString(Reason));
}

TArray<UPubnubMembership*> UPubnubUser::GetMemberships(int Limit, FString Start, FString End)
{
	TArray<UPubnubMembership*> PubnubMemberships;
	
	if(!IsInternalUserValid()) {return PubnubMemberships;}

	auto CppMemberships = InternalUser->get_memberships(Limit, UPubnubChatUtilities::FStringToPubnubString(Start), UPubnubChatUtilities::FStringToPubnubString(End));
	PubnubMemberships = UPubnubChatUtilities::CppMembershipsToUnrealMemberships(CppMemberships);
	
	return PubnubMemberships;
}

void UPubnubUser::StreamUpdates(FOnPubnubUserStreamUpdateReceived UserUpdateCallback)
{
	if(!IsInternalUserValid()) {return;}
	
	auto lambda = [UserUpdateCallback](Pubnub::User User)
	{
		AsyncTask(ENamedThreads::GameThread, [UserUpdateCallback, User]()
		{
			UPubnubUser* NewUser = UPubnubUser::Create(User);
			UserUpdateCallback.ExecuteIfBound(NewUser);
		});

	};
	InternalUser->stream_updates(lambda);
}

void UPubnubUser::StreamUpdatesOn(TArray<UPubnubUser*> Users, FOnPubnubUserStreamUpdateReceived UserUpdateCallback)
{
	if(!IsInternalUserValid()) {return;}

	auto lambda = [UserUpdateCallback](Pubnub::User User)
	{
		AsyncTask(ENamedThreads::GameThread, [UserUpdateCallback, User]()
		{
			UPubnubUser* NewUser = UPubnubUser::Create(User);
			UserUpdateCallback.ExecuteIfBound(NewUser);
		});

	};

	auto CppUsers = UPubnubChatUtilities::UnrealUsersToCppUsers(Users);

	InternalUser->stream_updates_on(CppUsers, lambda);
}

bool UPubnubUser::IsInternalUserValid()
{
	if(InternalUser == nullptr)
	{
		UE_LOG(PubnubLog, Error, TEXT("This PubnubUser is invalid"));
		return false;
	}
	return true;
}
