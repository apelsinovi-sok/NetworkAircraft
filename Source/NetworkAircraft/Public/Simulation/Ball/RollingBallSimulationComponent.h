#pragma once

#include "CoreMinimal.h"
#include "RollingBallPhysicsMovementTraits.h"
#include "Movement/PhysicsMovementComponent.h"

#include "RollingBallSimulationComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class NETWORKAIRCRAFT_API URollingBallSimulationComponent : public UPawnMovementComponent
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	virtual bool ShouldCreatePhysicsState() const override;
	virtual void OnCreatePhysicsState() override;
	virtual void OnDestroyPhysicsState() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void AsyncPhysicsTickComponent(float DeltaTime, float SimTime) override;

	void InitializeBall();
	void UpdateTargetAltitude(float DeltaTime);

	UFUNCTION(BlueprintCallable) void SetThrottleInput(float InThrottle);
	UFUNCTION(BlueprintCallable) void SetSteeringInput(float InSteering);
	UFUNCTION(BlueprintCallable) void SetTravelDirectionInput(FRotator InTravelDirection);
	UFUNCTION(BlueprintCallable) void Jump();
	UFUNCTION(BlueprintCallable) void SetThrottleUp(float Throttle);
	UFUNCTION(BlueprintCallable) void SwitchPidStatus();
	// UFUNCTION(BlueprintCallable) void SetThrottleDown(float Throttle);

	float GetPIDThrottle(float DeltaTime);

public:
	// реплицируемые инпуты/состояние (через NetworkPhysicsComponent)
	FBallInputs BallInputs;
	FBallState BallState;

	// локальные
	FTransform BallWorldTransform;
	
	UPROPERTY()
	TObjectPtr<UNetworkPhysicsComponent> NetworkPhysicsComponent = nullptr;

	Chaos::FRigidBodyHandle_Internal* RigidHandle = nullptr;
	FBodyInstance* BodyInstance = nullptr;

	int32 PreviousJumpCount = 0;
	int32 PreviousAltitudeVersion = 0;

	float PreviousPidThrottle = 0;

	FVector BallUpAxis = FVector::ZeroVector;

	// Локальные PID-переменные (не реплицируемые), но мы сохраняем часть в BallState
	float LocalErrorSum = 0.0f;
	float PidThrottle = 0.0f;
	float LastPidOutput = 0.0f;
	float CurrentAltitude = 0.0f;

	int32 PreviousPidEnableActiveStatus = 0;
	bool PreviousPidEnableActiveStatusBool = true;
};
