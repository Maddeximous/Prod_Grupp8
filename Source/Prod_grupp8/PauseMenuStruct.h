#pragma once

#include "CoreMinimal.h"
#include "PauseMenuStruct.generated.h"

UENUM(BlueprintType)
enum class EPauseMenuItemType : uint8
{
	Action,
	Submenu
};

UENUM(BlueprintType)
enum class EPauseMenuAction : uint8
{
	None,
	Resume,
	Restart,
	Quit,
	OpenSettings
};

USTRUCT(BlueprintType)
struct PROD_GRUPP8_API FPauseMenuItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPauseMenuItemType Type = EPauseMenuItemType::Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPauseMenuAction Action = EPauseMenuAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SubmenuIndex = -1;
};

USTRUCT(BlueprintType)
struct PROD_GRUPP8_API FMenuData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPauseMenuItem> Items;
};