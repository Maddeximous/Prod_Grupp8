
#pragma once

#include "ShopWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rod.h"
#include "Inventory.h"
#include "Shop.generated.h"

class AInventory;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AInventory* PlayerInventory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UShopWidget> ShopWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ARod*> Rods;
	
	int32 dollars= 0;
	
	void OpenShop();
	void addDollars();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	

private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	EShopSection CurrentSection = EShopSection::Inventory;

	int32 CurrentIndex = 0;

	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();

	void PrintCurrentItem();

};
