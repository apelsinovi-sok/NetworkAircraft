// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Physics/NetworkPhysicsComponent.h"

#include "FVehicleMovementTraits.generated.h"

/**
 * 
 */

USTRUCT()
struct NETWORKAIRCRAFT_API FVehicleSimulationInputs
{
	GENERATED_BODY()

public:
	FVehicleSimulationInputs()
	{
		ResetInputs();
	}
	~FVehicleSimulationInputs() {}

	UPROPERTY()
	double ThrottleInput_XAxis;

	UPROPERTY()
	double ThrottleInput_YAxis;

	UPROPERTY()
	double ThrottleInput_ZAxis;

	UPROPERTY()
	double YawInput;

	UPROPERTY()
	double PitchInput;

	UPROPERTY()
	double RollInput;

	UPROPERTY()
	double RawThrottleInputX;
	
	UPROPERTY()
	double RawThrottleInputY;
	
	UPROPERTY()
	double RawThrottleInputZ;

	UPROPERTY()
	double RawPitchInput;
	
	UPROPERTY()
	double RawRollInput;
	
	UPROPERTY()
	double RawYawInput;
	
	void ResetInputs();
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FDirectionState
{
	GENERATED_BODY()

public:
	FDirectionState() {}
	
	FDirectionState(FVector AxisDirection)
	{
		Axis = AxisDirection;
	}
	~FDirectionState() {}
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Axis = FVector::ZeroVector;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double Speed = 0.f;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double Acceleration = 0.f;

	void Update(FVector WorldVelocity, FVector WorldTransformAxis, double LocalAcceleration);
};

USTRUCT(BlueprintType)
struct NETWORKAIRCRAFT_API FVehicleSimulationState
{
	GENERATED_BODY()

public:

	FVehicleSimulationState()
	{
		Forward = FDirectionState(FVector::ForwardVector);
		Right = FDirectionState(FVector::RightVector);
		Up = FDirectionState(FVector::UpVector);
	}

	~FVehicleSimulationState() {}

	UPROPERTY()
	FTransform WorldTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WorldVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WorldAngularVelocity = FVector::ZeroVector;

	UPROPERTY()
	FVector WorldCOM = FVector::ZeroVector;

	UPROPERTY()
	FVector WorldVelocityNormal = FVector::ZeroVector;

	UPROPERTY()
	FVector LocalAcceleration = FVector::ZeroVector;

	UPROPERTY()
	FVector LocalGForce = FVector::ZeroVector;

	UPROPERTY()
	FVector LocalVelocity = FVector::ZeroVector;

	UPROPERTY()
	FVector LastFrameVehicleLocalVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDirectionState Forward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDirectionState Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDirectionState Up;

	/*
	 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CurrentThrottleValueX = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CurrentThrottleValueY = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CurrentThrottleValueZ = 0.f;

	void UpdateStateGT(const FBodyInstance* TargetInstance, double GravityZ, double deltaTime);

	void UpdateStatePT(const Chaos::FRigidBodyHandle_Internal* Handle, double GravityZ, double deltaTime);
};

USTRUCT()
struct NETWORKAIRCRAFT_API FVehicleNetPhysMoveStates : public FNetworkPhysicsData
{
	GENERATED_BODY()

public:	
	UPROPERTY()
	FVehicleSimulationState MovementStates;

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
struct TStructOpsTypeTraits<FVehicleNetPhysMoveStates> : public TStructOpsTypeTraitsBase2<FVehicleNetPhysMoveStates>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct NETWORKAIRCRAFT_API FVehicleNetPhysMoveInputs : public FNetworkPhysicsData
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FVehicleSimulationInputs MovementInputs;

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
struct TStructOpsTypeTraits<FVehicleNetPhysMoveInputs> : public TStructOpsTypeTraitsBase2<FVehicleNetPhysMoveInputs>
{
	enum
	{
		WithNetSerializer = true,
	};
};

struct NETWORKAIRCRAFT_API FVehicleMovementTraits
{
	using InputsType = FVehicleNetPhysMoveInputs;
	using StatesType = FVehicleNetPhysMoveStates;
};


