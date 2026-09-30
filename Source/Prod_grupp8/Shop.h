
#pragma once

#include "ShopWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rod.h"
#include "Shop.generated.h"

UCLASS()
class PROD_GRUPP8_API AShop : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShop();
	
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
	
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
