#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EPubnubMessageActionType : uint8
{
	PMAT_Reaction			 UMETA(DisplayName="Reaction"),
	PMAT_Receipt			 UMETA(DisplayName="Receipt"),
	PMAT_Custom				 UMETA(DisplayName="Custom"),
	PMAT_Edited				 UMETA(DisplayName="Edited"),
	PMAT_Deleted			 UMETA(DisplayName="Deleted")
};

UENUM(BlueprintType)
enum class EPubnubChatEventType : uint8
{
	PCET_TYPING				 UMETA(DisplayName="Typing"),
	PCET_REPORT				 UMETA(DisplayName="Report"),
	PCET_RECEPIT			 UMETA(DisplayName="Receipt"),
	PCET_MENTION			 UMETA(DisplayName="Mention"),
	PCET_INVITE				 UMETA(DisplayName="Invite"),
	PCET_CUSTOM				 UMETA(DisplayName="Custom"),
	PCET_MODERATION			 UMETA(DisplayName="Moderation")
};

UENUM(BlueprintType)
enum class EPubnubChatMessageType : uint8
{
	PCMT_TEXT			 UMETA(DisplayName="Text")
};