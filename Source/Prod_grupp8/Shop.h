
#pragma once

#include "ShopWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rod.h"
#include "Inventory.h"
#include "Shop.generated.h"

class AInventory;
class USphereComponent;

UENUM()
enum class EShopSection
{
	Inventory,
	Shop
};

UCLASS()
class PROD_GRUPP8_API AShop : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShop();
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool shopEnabled = true;
	
	bool previousShopEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AInventory* PlayerInventory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UShopWidget> ShopWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* ShopSphere;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ARod*> Rods;
	
	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	int32 dollars= 0;
	
	void ToggleShop();
	void addDollars();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	

private:	
	// Called every frame
	
	EShopSection CurrentSection = EShopSection::Inventory;

	int32 CurrentIndex = 0;
	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();
	
	void Transaction();

	void PrintCurrentItem();

};
