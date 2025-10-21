// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SimulationConfigurations.generated.h"

/**
 * 
 */

UENUM(Blueprintable)
enum EInputControlType : uint8
{
	ICT_Throttle_X    UMETA(DisplayName = "Throttle Input on X Axis"),
	ICT_Throttle_Y    UMETA(DisplayName = "Throttle Input on Y Axis"),
	ICT_Throttle_Z    UMETA(DisplayName = "Throttle Input on Z Axis"),
	ICT_Input_Roll    UMETA(DisplayName = "Roll Input"),
	ICT_Input_Pitch   UMETA(DisplayName = "Pitch Input"),
	ICT_Input_Yaw     UMETA(DisplayName = "Yaw Input")
};

UENUM(Blueprintable)
enum EForceControlAxis : uint8
{
	FCA_Static_X    UMETA(DisplayName = "Along X Axis"),
	FCA_Static_Y    UMETA(DisplayName = "Along Y Axis"),
	FCA_Static_Z    UMETA(DisplayName = "Along Z Axis")
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FForceDirectionConfiguration
{
	GENERATED_BODY()

public:

	FForceDirectionConfiguration() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EForceControlAxis> ThrustAxis = EForceControlAxis::FCA_Static_X;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double PitchAngleOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double RollAngleOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double YawAngleOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsDirectionNegated = false;

	FVector GetThrustDirection();
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FForceLocationConfiguration
{
	GENERATED_BODY()

public:

	FForceLocationConfiguration() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BoneOrSocketName;

	/*Used to offset off the ThrustPoint*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LocationOffset = FVector::ZeroVector;

	void SetForceLocation(FVector location) { SocketOrBoneLocation = location; };

	FVector GetLocation() const { return SocketOrBoneLocation + LocationOffset; };

private:

	FVector SocketOrBoneLocation = FVector::ZeroVector;
};


USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FPhysicsThrustForce
{
	GENERATED_BODY()

public:

	FPhysicsThrustForce() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* ThrustCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double MaxThrust = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double MaxThrustMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, ClampMax = 1.f))
	double ThrustConditionPercent = 1.0f;

	double GetMaxThrust(double altitude);
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FPhysicsStaticThrusterSetup
{
	GENERATED_BODY()

public:
	FPhysicsStaticThrusterSetup() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TagName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bThrusterEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EInputControlType> ControlInputSource = EInputControlType::ICT_Throttle_X;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FForceDirectionConfiguration ThrustDirection;

	/*Can Be a Bone or Socket on the Mesh*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FForceLocationConfiguration ThrustPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPhysicsThrustForce ThrusterForces;

	void SetThrottle(float input);

	void SetWorldAltitude(const double altitude) { Altitude = altitude; }

	const FVector GetForce();

private:
	double Altitude = 0.f;
	double ThrustInput = 0.f;
};