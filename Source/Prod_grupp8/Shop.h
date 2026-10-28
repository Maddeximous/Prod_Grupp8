#pragma once

#include "CoreMinimal.h"
#include "FishManager.h"
#include "GameFramework/Actor.h"
#include "ShopWidget.h"
#include "Rod.h"
#include "Inventory.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "NumberSpeaker.h"
#include "Shop.generated.h"

class AInventory;

UCLASS()
class PROD_GRUPP8_API AShop : public AActor
{
	GENERATED_BODY()

public:
	AShop();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool shopEnabled = false;

	bool bCanOpenShop = false;
	
	UPROPERTY()
	TObjectPtr<AFishManager> FishManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AInventory* PlayerInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UShopWidget> ShopWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	// Rods for sale, in the order they are unlocked. The shop always sells Rods[NextRodIndex].
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ARod*> Rods;

	// Index into Rods of the next rod the player can buy.
	// Set to 1 in the editor if Rods[0] is a starter rod the player already owns.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 NextRodIndex = 0;

	// ------------ Shop sounds -------------------
	
	// Ljudkomponenten som sköter uppspelningen
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<UAudioComponent> ShopAudioComponent;

	// Själva ljudet (t.ex. bakgrundsmusik eller miljöbjäfs i shoppen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> ShopAmbientSound;
	
	// Sälj sfx
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SellSound;
	
	// Köp sfx
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> BuySound;
	
	// No money sfx
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> NoMoneySound;
	
	// Dialog för "Köpte"
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* BoughtDialogueSound;
	
	// Dialog för "För"
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* ForDialogueSound;
	
	// Dialog för "Sålde"
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* SoldDialogueSound;
	
	// Dialog för "Fiskar För"
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* FishForDialogueSound;
	
	// Dialog för "Kostar"
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* CostsDialogueSound;
	
	// Dialog för "Du har"
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* YouHaveDialogueSound;
	
	//Ljud som kallar på numberspeaker klassen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio")
	TObjectPtr<ANumberSpeaker> NumberSpeaker;

	UFUNCTION()
	void OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	int32 dollars = 0;

	void OpenShop();
	void CloseShop();
	void ToggleShop();

	// Shop actions
	void SellAllFish();
	void BuyNextRod();

	// Debug: +100 dollars
	void addDollars();

	// Legacy entry points still called by AProd_grupp8Character (DoShopTransaction / DoSwitchShopView).
	// They intentionally do nothing now; remove them once the character no longer calls them.
	void Transaction();
	void ToggleSection();
	
	
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Audio Functions")
	void OnFishSold(const int FishSold, const int TotalPrice);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Audio Functions")
	void OnRodBought(const int RodIndex, const int BoughtFor);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Audio Functions")
	void OnRodCantBuy(const int RodIndex, const int RodPrice, const int MoneyOnPlayer);
	
	UFUNCTION(BlueprintImplementableEvent,  Category = "Audio Functions")
	void OnEnterShop();
	
	// A helper method that takes a delay and handles the timer internally
	void PlayNoMoneySoundWithDelay(float DelaySeconds = 1.0f);
	
	void BoughtRodSound(const ARod* Rod, int32 Price);
	void SoldFishSound(int32 FishSold, int32 TotalPrice);
	void CantBuyRodSound(const ARod* Rod, int32 Price, int32 CurrentMoney);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidget;

	void RefreshWidget();
	void ShowMoney() const;
	void ShowMessage(int32 Key, float Time, const FColor& Color, const FString& Text) const;
};