// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "FishManager.h"
#include "PauseMenu.h"
#include "Inventory.h"
#include "Shop.h"
#include "Prod_grupp8Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class AProd_grupp8Character : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;
	
	/** Cast Line Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* CastLineAction;
	
	/** Reel Line Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* ReelLineAction;
	
	/** Sonar Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* SonarAction;
	
	/** Sonar Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* SwayRightAction;
	
	/** Sonar Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* SwayLeftAction;
	
	/** Sonar Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* OpenPauseMenuAction;
	
	/** Shop Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* ShopTransactionAction;
	
	/** Switch Shop View Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* SwitchShopViewAction;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* ThrowSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* OnHookSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* ReelInSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* CaughtSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* SonarSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Haptics")
	class UForceFeedbackEffect* ReelHapticEffect;
	
	UPROPERTY(EditAnywhere, Category ="Haptics")
	class UForceFeedbackEffect* FightHapticEffect;
	
	UPROPERTY()
	UAudioComponent* ReelInSoundCueObject;
	
	/** Looping sound that plays constantly; its pitch follows the player's forward acceleration */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	class USoundBase* MoveForwardSoundCue;
	
	/** Pitch while the player is stationary */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float IdleMovePitch = 0.5f;
	
	/** Pitch when moving forward with very little acceleration */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float MinMovePitch = 0.8f;
	
	/** Pitch at full acceleration */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float MaxMovePitch = 1.4f;
	
	/** How quickly the pitch follows changes in acceleration */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float MovePitchInterpSpeed = 8.0f;
	
	/** Extra pitch added at full turn speed when turning left while standing still */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float TurnLeftPitchBoost = 0.15f;

	/** Extra pitch added at full turn speed when turning right while standing still (negative values lower the pitch) */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float TurnRightPitchBoost = 0.25f;

	/** Turn speed (degrees/sec) that gives the full left/right turn boost */
	UPROPERTY(EditAnywhere, Category ="Audio|Movement")
	float MaxTurnRateForPitch = 180.0f;

	UPROPERTY()
	UAudioComponent* MoveForwardSoundObject;

	float CurrentMovePitch = 0.5f;

	float PreviousYaw = 0.0f;
	
	void UpdateMoveSound(float DeltaTime);
	
	bool bIsFishing = false;
	
	bool bIsFishOnHook = false;
	
	bool bFishIsFighting = false;
	
	bool bReelSoundActive = false;
	
	FVector2d ReelStickPosition = FVector2d::ZeroVector;

	float ReelStickDegree = 0.0f;
	
	float caughtProgress = 0.0f;
	
	float FishFightTimer = 0.0f;
	
	float FishFightScore = 0.0f;
	
	float ReelSoundTimer = 0.0f;
	
	float ProgressToCatch = 0.0f;
	
	UPROPERTY()
	TObjectPtr<AActor> Bobber;
	
	UPROPERTY()
	TObjectPtr<AFishManager> FishManager;
	
	UPROPERTY()
	TObjectPtr<APauseMenu> PauseMenu;
	
	UPROPERTY()
	TObjectPtr<AInventory> Inventory;

	
public:
	AProd_grupp8Character();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int FishCaught = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor> BobberToSpawn;
	
	UPROPERTY()
	TObjectPtr<AFish> FishInRange;
	
	UPROPERTY()
	TObjectPtr<AShop> ShopInRange;
	
	

protected:
	virtual void BeginPlay() override;

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);
	
	/** Called from Input Actions for reel line input */
	void ReelLineInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles castLine start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCastLine();
	
	/** Handles sonar input */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSonar();
	
	/** Handles transaction input */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoShopTransaction();
	
	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	/** Handles Sway input for LB (right) */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSwayLeft();
	
	/** Handles Sway input for RB (left) */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSwayRight();
	
	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoReelLine(float Right, float Down);
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoOpenPauseMenu();
	
	/** Switch Shop view between rods and fish */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSwitchShopView();
	
	void StartFishing();
	
	void OnHook();
	
	void Caught();
	
	AActor* SpawnBobber(FVector SpawnLocation);
	
	void StartFishFighting();
	
	float Reeling(float DeltaTime);
	
	void Fishing(float DeltaTime, float ReelValue);
	
	void FishFighting(float DeltaTime, float ReelValue);

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	virtual void Tick(float DeltaTime) override;

};