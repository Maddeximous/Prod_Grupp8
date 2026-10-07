#pragma once

#include "CoreMinimal.h"
#include "PauseMenuStruct.h"
#include "GameFramework/Actor.h"
#include "PauseMenu.generated.h"

UCLASS()
class PROD_GRUPP8_API APauseMenu : public AActor
{
	GENERATED_BODY()

public:

	APauseMenu();

	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pause Menu")
	bool bPaused = false;

	bool bPreviousPaused = false;
	
	bool IsPaused() const;

	
private:
	
	int32 CurrentMenuIndex = 0;

	int32 SelectedItemIndex = 0;
	
	TArray<int32> MenuHistory;
	
	void TogglePauseMenu();
	
	void MoveUp();
	void MoveDown();
	void Select();
	void GoBack();
	void ExecuteAction(EPauseMenuAction Action);
	
	void HandleUp();
	void HandleDown();
	void HandleSelect();
	void HandleBack();

	void UpdateDebugText();
	void VoiceState();
	
	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMenuData> Menus;
	virtual void BeginPlay() override;
};