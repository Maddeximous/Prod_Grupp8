// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
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
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* ThrowSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* OnHookSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* ReelInSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Audio")
	class USoundBase* CaughtSoundCue;
	
	UPROPERTY(EditAnywhere, Category ="Haptics")
	class UForceFeedbackEffect* FishHapticEffect;
	
	
	bool bIsFishing = false;
	
	bool bIsFishOnHook = false;
	
	bool bFishIsFighting = false;
	
	FVector2d ReelStickPosition;

	float ReelStickDegree;
	
	float caughtProgress = 0.0f;
	
	float FishFightTimer = 0.0f;

	UPROPERTY()
	TObjectPtr<AActor> Bobber;
	
public:
	AProd_grupp8Character();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor> BobberToSpawn;
	
	UPROPERTY()
	TObjectPtr<AFish> FishInRange;

protected:

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
	
	/** Handles Sway input for LB (right) */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSwayLeft();
	
	/** Handles Sway input for RB (left) */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSwayRight();
	
	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoReelLine(float Right, float Down);
	
	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	void StartFishing();
	
	void OnHook();
	
	void Caught();
	
	// bool IsReeling();
	
	void Fishing(float deltaTime);
	
	AActor* SpawnBobber(FVector SpawnLocation);
	
	void StartFishFighting();
	
	void FishFighting(float DeltaTime);
	
	void FishFightingLeft(float DeltaTime);
	
	void FishFightingRight(float DeltaTime);

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

