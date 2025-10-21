// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RollingBallPhysicsMovementTraits.h"
#include "Movement/PhysicsMovementComponent.h"

#include "RollingBallSimulationComponent.generated.h"

/**
 * 
 */

UCLASS(meta = (BlueprintSpawnableComponent))
class NETWORKAIRCRAFT_API URollingBallSimulationComponent : public UPawnMovementComponent
{
	GENERATED_UCLASS_BODY()
	
public:

	/** Overridden to allow registration with components NOT owned by a Pawn. */
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	/** Return true if it's suitable to create a physics representation of the Ball at this time */
	virtual bool ShouldCreatePhysicsState() const override;

	/** Used to create any physics engine information for this component */
	virtual void OnCreatePhysicsState() override;

	/** Used to shut down and physics engine structure for this component */
	virtual void OnDestroyPhysicsState() override;

	void InitializeBall();

	virtual void AsyncPhysicsTickComponent(float DeltaTime, float SimTime);

	UFUNCTION(BlueprintCallable)
	void SetThrottleInput(float InThrottle);

	UFUNCTION(BlueprintCallable)
	void SetSteeringInput(float InSteering);

	UFUNCTION(BlueprintCallable)
	void SetTravelDirectionInput(FRotator InTravelDirection);

	UFUNCTION(BlueprintCallable)
	void Jump();

public:

	// Ball inputs
	FBallInputs BallInputs;

	// Ball state
	FTransform BallWorldTransform;
	FVector BallForwardAxis;
	FVector BallRightAxis;

private:
	
	UPROPERTY()
	TObjectPtr<UNetworkPhysicsComponent> NetworkPhysicsComponent = nullptr;

	Chaos::FRigidBodyHandle_Internal* RigidHandle;
	FBodyInstance* BodyInstance;
	int32 PreviousJumpCount = 0;
};
