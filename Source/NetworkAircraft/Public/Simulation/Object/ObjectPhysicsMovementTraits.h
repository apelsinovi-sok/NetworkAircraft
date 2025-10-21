// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Physics/NetworkPhysicsComponent.h"

#include "ObjectPhysicsMovementTraits.generated.h"

/**
 * 
 */

USTRUCT()
struct NETWORKAIRCRAFT_API FObjectSimulationInputs
{
	GENERATED_BODY()

public:
	FObjectSimulationInputs()
	: ThrottleInput_XAxis(0.f)
	, ThrottleInput_YAxis(0.f)
	, YawInput(0.f)
	, BoostCount(0)
	{}

	~FObjectSimulationInputs() {}

	UPROPERTY()
	float ThrottleInput_XAxis;

	UPROPERTY()
	float ThrottleInput_YAxis;

	UPROPERTY()
	float YawInput;

	UPROPERTY()
	int32 BoostCount;

	void ResetInputs();
};

USTRUCT()
struct NETWORKAIRCRAFT_API FObjectSimulationState
{
	GENERATED_BODY()

public:
	FObjectSimulationState()
	: TravelDirection(FRotator::ZeroRotator)
	, PreviousJumpCount(0)
	{}
    
	~FObjectSimulationState() {}

	UPROPERTY()
	FRotator TravelDirection;

	UPROPERTY()
	int PreviousJumpCount;
};

USTRUCT()
struct NETWORKAIRCRAFT_API FObjectNetPhysMoveStates : public FNetworkPhysicsData
{
public:
	GENERATED_BODY()

	UPROPERTY()
	FObjectSimulationState MovementStates;

	/** Apply the data onto the network physics component */
	virtual void ApplyData(UActorComponent* NetworkComponent) const override;

	/** Build the data from the network physics component */
	virtual void BuildData(const UActorComponent* NetworkComponent) override;

	/** Decay input during resimulation */
	virtual void DecayData(float DecayAmount) override;

	/** Serialize data function that will be used to transfer the struct across the network */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	/** Interpolate the data in between two inputs data */
	virtual void InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData) override;

	/** Merge data into this input */
	virtual void MergeData(const FNetworkPhysicsData& FromData) override;
};

template<>
struct TStructOpsTypeTraits<FObjectNetPhysMoveStates> : public TStructOpsTypeTraitsBase2<FObjectNetPhysMoveStates>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct NETWORKAIRCRAFT_API FObjectNetPhysMoveInputs : public FNetworkPhysicsData
{
	GENERATED_BODY()

	/** List of incoming control inputs coming from the local client */
	UPROPERTY()
	FObjectSimulationInputs MovementInputs;

	/** Apply the data onto the network physics component */
	virtual void ApplyData(UActorComponent* NetworkComponent) const override;

	/** Build the data from the network physics component */
	virtual void BuildData(const UActorComponent* NetworkComponent) override;

	/** Decay input during resimulation */
	virtual void DecayData(float DecayAmount) override;

	/** Serialize data function that will be used to transfer the struct across the network */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	/** Interpolate the data in between two inputs data */
	virtual void InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData) override;

	/** Merge data into this input */
	virtual auto MergeData(const FNetworkPhysicsData& FromData) -> void override;
};

template<>
struct TStructOpsTypeTraits<FObjectNetPhysMoveInputs> : public TStructOpsTypeTraitsBase2<FObjectNetPhysMoveInputs>
{
	enum
	{
		WithNetSerializer = true,
	};
};

struct NETWORKAIRCRAFT_API FObjectPhysicsMovementTraits
{
	using InputsType = FObjectNetPhysMoveInputs;
	using StatesType = FObjectNetPhysMoveStates;
};
