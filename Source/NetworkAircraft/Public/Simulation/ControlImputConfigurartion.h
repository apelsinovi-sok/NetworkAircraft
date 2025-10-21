// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

#include "ControlImputConfigurartion.generated.h"



USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FInputRateConfiguration
{
	GENERATED_BODY()
public:
	FInputRateConfiguration() {}
	~FInputRateConfiguration() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnable = true;
	
	UPROPERTY(EditAnywhere, Category = VehicleInputRate)
	double RiseRate = 5.f;
	
	UPROPERTY(EditAnywhere, Category = VehicleInputRate)
	double FallRate = 5.f;
	
	double GetInterpilatedValue(double DeltaTime, double CurrentValue, double Target);
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FThrottleControlConfiguration
{
	GENERATED_BODY()
public:
	FThrottleControlConfiguration() {}
	~FThrottleControlConfiguration() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnableThrottleControl = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanReverseThrottle = false;
	
	UPROPERTY(EditAnywhere, Category = VehicleInputRate, meta = (ClampMin = 0.001, ClampMax = 1.f))
	double RiseRate = 0.01f;
	
	UPROPERTY(EditAnywhere, Category = VehicleInputRate)
	double InterpRise = 2.f;
	
	UPROPERTY(EditAnywhere, Category = VehicleInputRate, meta = (ClampMin = 0.001, ClampMax = 1.f))
	double FallRate = 0.01f;
	
	UPROPERTY(EditAnywhere, Category = VehicleInputRate)
	double InterpFall = 1.f;
	
	double GetThrottleValue(double value, double deltaSeconds);
	
	double GetCurrentThrottleValue() const { return CurrentThrottleValue; }
	
	void SetCurrentThrottleValue(double value) { CurrentThrottleValue = value; }

private:
	double CurrentThrottleValue = 0.0f;
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FThrottleConfiguration
{
	GENERATED_BODY()
public:
	FThrottleConfiguration() {}
	~FThrottleConfiguration() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInputRateConfiguration ThrottleInputRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FThrottleControlConfiguration ThrottleControl;

	double GetInterpilatedValue(double deltaTime, double CurrentValue, double RawValue);
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FPhysicsDampingConfiguration
{
	GENERATED_BODY()
public:
	FPhysicsDampingConfiguration() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnabled = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Damping = 0.02f;
	
	void AddDampingEffect(const FVector worldVelocity, FVector& outForce);
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FInputScalingConfiguration
{
	GENERATED_BODY()
public:
	FInputScalingConfiguration() {}
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double Scaling = 1.0f;
	
	FVector GetTorque(const double controlInput, const FVector& Axis);
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FPhysicsTorqueConfiguration
{
	GENERATED_BODY()
public:
	FPhysicsTorqueConfiguration() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInputScalingConfiguration YawInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInputScalingConfiguration YawFromRollInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInputScalingConfiguration PitchInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInputScalingConfiguration RollInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPhysicsDampingConfiguration TorqueDamping;
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FPhysicsSimpleRotationConfiguration
{
	GENERATED_BODY()

public:
	FPhysicsSimpleRotationConfiguration() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnableRotationControl = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPhysicsTorqueConfiguration TorqueInputScaling;
};