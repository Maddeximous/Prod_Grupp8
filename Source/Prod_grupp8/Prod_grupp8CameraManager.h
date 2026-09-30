// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Prod_grupp8CameraManager.generated.h"

/**
 *  Basic First Person camera manager.
 *  Limits min/max look pitch.
 */
UCLASS()
class AProd_grupp8CameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AProd_grupp8CameraManager();
};
