#pragma once

#include "CoreMinimal.h"
#include "Physics/NetworkPhysicsComponent.h"

#include "RollingBallPhysicsMovementTraits.generated.h"

USTRUCT()
struct NETWORKAIRCRAFT_API FBallInputs
{
	GENERATED_BODY()

	FBallInputs()
		: SteeringInput(0.f)
		, ThrottleInput(0.f)
		, TravelDirection(FRotator::ZeroRotator)
		, JumpCount(0)
		, ThrottleUp(0)
		, TargetAltitude(1000.f)
		, AltitudeVersion(0)
		, PidEnableActiveStatus(0)
	{}

	UPROPERTY()
	float SteeringInput;

	UPROPERTY()
	float ThrottleInput;

	UPROPERTY()
	FRotator TravelDirection;

	UPROPERTY()
	int32 JumpCount;

	UPROPERTY()
	float ThrottleUp;

	UPROPERTY()
	float TargetAltitude;

	UPROPERTY()
	int32 AltitudeVersion;

	UPROPERTY()
	int32 PidEnableActiveStatus;
};

USTRUCT()
struct NETWORKAIRCRAFT_API FBallState
{
	GENERATED_BODY()

	FBallState()
		: BallForwardAxis(FVector::ZeroVector)
		, BallRightAxis(FVector::ZeroVector)
		, ThrottleUp(0)
		, WorldVelocity(FVector::ZeroVector)
		, BallUpAxis(FVector::ZeroVector)
		, CurrentAltitude(0)
		, ErrorSum(0)
		, PidThrottle(0)
	{}

	UPROPERTY()
	FVector BallForwardAxis;

	UPROPERTY()
	FVector BallRightAxis;

	UPROPERTY()
	float ThrottleUp;

	UPROPERTY()
	FVector WorldVelocity;

	UPROPERTY()
	FVector BallUpAxis;

	UPROPERTY()
	float CurrentAltitude;

	// Добавлено: состояние PID для корректной рессимуляции
	UPROPERTY()
	float ErrorSum;

	// Добавлено: текущий выход PID (сохраняем, чтобы при rewind не прыгал)
	UPROPERTY()
	float PidThrottle;
};

USTRUCT()
struct NETWORKAIRCRAFT_API FNetworkBallStates : public FNetworkPhysicsData
{
	GENERATED_BODY()

	UPROPERTY()
	FBallState BallState;

	virtual void ApplyData(UActorComponent* NetworkComponent) const override;
	virtual void BuildData(const UActorComponent* NetworkComponent) override;
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	virtual void InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData) override;
	virtual void MergeData(const FNetworkPhysicsData& FromData) override;
};

template<>
struct TStructOpsTypeTraits<FNetworkBallStates> : public TStructOpsTypeTraitsBase2<FNetworkBallStates>
{
	enum { WithNetSerializer = true, };
};

USTRUCT()
struct NETWORKAIRCRAFT_API FNetworkBallInputs : public FNetworkPhysicsData
{
	GENERATED_BODY()

	UPROPERTY()
	FBallInputs BallInputs;

	virtual void ApplyData(UActorComponent* NetworkComponent) const override;
	virtual void BuildData(const UActorComponent* NetworkComponent) override;
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	virtual void InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData) override;
	virtual void MergeData(const FNetworkPhysicsData& FromData) override;
};

template<>
struct TStructOpsTypeTraits<FNetworkBallInputs> : public TStructOpsTypeTraitsBase2<FNetworkBallInputs>
{
	enum { WithNetSerializer = true, };
};

struct NETWORKAIRCRAFT_API FRollingBallPhysicsMovementTraits
{
	using InputsType = FNetworkBallInputs;
	using StatesType = FNetworkBallStates;
};
