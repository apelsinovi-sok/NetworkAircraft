// Copyright Epic Games, Inc. All Rights Reserved.


#include "NetworkAircraftPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "NetworkAircraftCameraManager.h"

ANetworkAircraftPlayerController::ANetworkAircraftPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = ANetworkAircraftCameraManager::StaticClass();
}

void ANetworkAircraftPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
