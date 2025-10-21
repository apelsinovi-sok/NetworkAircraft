// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

namespace Chaos
{
	class FRigidBodyHandle_Internal;
}

struct NETWORKAIRCRAFT_API FAddForceData
{
public:
	FAddForceData(const FVector& ForceInput, bool IsAccelerationChange)
	{
		Force = ForceInput;
		bIsAccelerationChange = IsAccelerationChange;
	}
	~FAddForceData() {}

	FVector Force;
	bool bIsAccelerationChange = false;
};

struct NETWORKAIRCRAFT_API FAddForceAtPositionData
{
public:
	FAddForceAtPositionData(const FVector& ForceInput, const FVector& ForcePosition)
	{
		Force = ForceInput;
		Position = ForcePosition;
	}
	~FAddForceAtPositionData() {}

	FVector Force;
	FVector Position;
};

struct NETWORKAIRCRAFT_API FAddTorqueData
{
public:
	FAddTorqueData(const FVector& TorqueInRadians, bool IsAccelerationChange)
	{
		Torque = TorqueInRadians;
		bIsAccelerationChange = IsAccelerationChange;
	}
	~FAddTorqueData() {}
    
	FVector Torque;
	bool bIsAccelerationChange = false;
};

struct NETWORKAIRCRAFT_API FAddImpulseData
{
public:
	FAddImpulseData(const FVector& ImpulseInput, bool IsVelocityChange)
	{
		Impulse = ImpulseInput;
		bIsVelocityChange = IsVelocityChange;
	}
	~FAddImpulseData() {}

	FVector Impulse;
	bool bIsVelocityChange = false;
};

struct NETWORKAIRCRAFT_API FAddImpulseAtPositionData
{
public:
	FAddImpulseAtPositionData(const FVector& ImpulseInput, const FVector& ImpulsePosition)
	{
		Impulse = ImpulseInput;
		Position = ImpulsePosition;
	}
	~FAddImpulseAtPositionData() {}

	FVector Impulse;
	FVector Position;
};

struct NETWORKAIRCRAFT_API FDeferredForcesLoader
{
public:
	FDeferredForcesLoader() {}
	~FDeferredForcesLoader() {}
	void Add(const FAddForceData& value) { ApplyForces.Add(value); }
	void Add(const FAddForceAtPositionData& value) { ApplyForcesAtPosition.Add(value); }
	void Add(const FAddTorqueData& value) { ApplyTorques.Add(value); }
	void Add(const FAddImpulseData& value) { ApplyImpulses.Add(value); }
	void Add(const FAddImpulseAtPositionData& value) { ApplyAddImpulsesAtPosition.Add(value); }
	void Apply(Chaos::FRigidBodyHandle_Internal* RigidHandle);
	void Reset();
	void UnLock();
	bool IsLocked() const;
    
private:
	bool bIsLocked = true;
	TArray<FAddForceData> ApplyForces;
	TArray<FAddForceAtPositionData> ApplyForcesAtPosition;
	TArray<FAddTorqueData> ApplyTorques;
	TArray<FAddImpulseData> ApplyImpulses;
	TArray<FAddImpulseAtPositionData> ApplyAddImpulsesAtPosition;

	void AddForce(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddForceData& Data);
	void AddForceAtPosition(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddForceAtPositionData& Data);
	void AddTorque(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddTorqueData& Data);
	void AddImpulse(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddImpulseData& Data);
	void AddImpulseAtPosition(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddImpulseAtPositionData& Data);
};
