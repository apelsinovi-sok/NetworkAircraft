// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPhysicsMovementTraits.h"
#include "Movement/PhysicsMovementComponent.h"

#include "ObjectSimulationComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class NETWORKAIRCRAFT_API UObjectSimulationComponent : public UPhysicsMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FObjectSimulationInputs SimulationInputs;
	
	UPROPERTY()
	FObjectSimulationState SimulationState;

protected:
	/*Hook into the OnCreatePhysicsState*/
	virtual void InitializeNetworkPhysicsMovement() override;

	/*Hook into the OnCreatePhysicsState*/
	virtual void SimulatePhysicsTick(double DeltaTime, Chaos::FRigidBodyHandle_Internal* Handle) override;

public:
	UFUNCTION(BlueprintCallable)
	void SetThrottleInputAxisX(double value);

	UFUNCTION(BlueprintCallable)
	void SetThrottleInputAxisY(double value);

	UFUNCTION(BlueprintCallable)
	void SetYawInput(double value);

	UFUNCTION(BlueprintCallable)
	void ImpulseBoost();
};
