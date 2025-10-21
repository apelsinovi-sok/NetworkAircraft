// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/Object/ObjectPhysicsMovementTraits.h"
#include "Simulation/Object/ObjectSimulationComponent.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

void FObjectSimulationInputs::ResetInputs()
{
	ThrottleInput_XAxis = 0.0f;
	ThrottleInput_YAxis = 0.0f;
	YawInput=0.f;
}

void FObjectNetPhysMoveStates::ApplyData(UActorComponent* NetworkComponent) const
{
	if (UObjectSimulationComponent* PhysicsMover = Cast<UObjectSimulationComponent>(NetworkComponent))
	{
		PhysicsMover->SimulationState = MovementStates;
	}
}

void FObjectNetPhysMoveStates::BuildData(const UActorComponent* NetworkComponent)
{
	if (NetworkComponent)
	{
		if (const UObjectSimulationComponent* PhysicsMover = Cast<const UObjectSimulationComponent>(NetworkComponent))
		{
			MovementStates = PhysicsMover->SimulationState;
		}
	}
}

void FObjectNetPhysMoveStates::DecayData(float DecayAmount)
{
	DecayAmount = FMath::Min(DecayAmount * 2, 1.0f);
}

bool FObjectNetPhysMoveStates::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FNetworkPhysicsData::SerializeFrames(Ar);
	return true;
}

void FObjectNetPhysMoveStates::InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData)
{
}

void FObjectNetPhysMoveStates::MergeData(const FNetworkPhysicsData& FromData)
{
	InterpolateData(FromData, *this);
}



void FObjectNetPhysMoveInputs::ApplyData(UActorComponent* NetworkComponent) const
{
	if (UObjectSimulationComponent* PhysicsMover = Cast<UObjectSimulationComponent>(NetworkComponent))
	{
		PhysicsMover->SimulationInputs = MovementInputs;
	}
}

void FObjectNetPhysMoveInputs::BuildData(const UActorComponent* NetworkComponent)
{
	if (NetworkComponent)
	{
		if (const UObjectSimulationComponent* PhysicsMover = Cast<const UObjectSimulationComponent>(NetworkComponent))
		{
			MovementInputs = PhysicsMover->SimulationInputs;
		}
	}
}

void FObjectNetPhysMoveInputs::DecayData(float DecayAmount)
{
	DecayAmount = FMath::Min(DecayAmount * 2, 1.0f);

	MovementInputs.ThrottleInput_XAxis = FMath::Lerp(MovementInputs.ThrottleInput_XAxis, 0.0f, DecayAmount);
	MovementInputs.ThrottleInput_YAxis = FMath::Lerp(MovementInputs.ThrottleInput_YAxis, 0.0f, DecayAmount);
	MovementInputs.YawInput = FMath::Lerp(MovementInputs.YawInput, 0.0f, DecayAmount);
}

bool FObjectNetPhysMoveInputs::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FNetworkPhysicsData::SerializeFrames(Ar);

	Ar << MovementInputs.ThrottleInput_XAxis;
	Ar << MovementInputs.ThrottleInput_YAxis;
	Ar << MovementInputs.YawInput;
	Ar << MovementInputs.BoostCount;

	bOutSuccess = true;
	return bOutSuccess;
}

void FObjectNetPhysMoveInputs::InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData)
{
	const FObjectNetPhysMoveInputs& MinInput = static_cast<const FObjectNetPhysMoveInputs&>(MinData);
	const FObjectNetPhysMoveInputs& MaxInput = static_cast<const FObjectNetPhysMoveInputs&>(MaxData);

	const float LerpFactor = MaxInput.LocalFrame == LocalFrame
		? 1.0f / (MaxInput.LocalFrame - MinInput.LocalFrame + 1) // Merge from min into max
		: (LocalFrame - MinInput.LocalFrame) / (MaxInput.LocalFrame - MinInput.LocalFrame); // Interpolate from min to max

	MovementInputs.ThrottleInput_XAxis = FMath::Lerp(MinInput.MovementInputs.ThrottleInput_XAxis, MaxInput.MovementInputs.ThrottleInput_XAxis, LerpFactor);
	MovementInputs.ThrottleInput_YAxis = FMath::Lerp(MinInput.MovementInputs.ThrottleInput_YAxis, MaxInput.MovementInputs.ThrottleInput_YAxis, LerpFactor);
	MovementInputs.YawInput = FMath::Lerp(MinInput.MovementInputs.YawInput, MaxInput.MovementInputs.YawInput, LerpFactor);
	MovementInputs.BoostCount = LerpFactor < 0.5 ? MinInput.MovementInputs.BoostCount : MaxInput.MovementInputs.BoostCount;
}

void FObjectNetPhysMoveInputs::MergeData(const FNetworkPhysicsData& FromData)
{
	InterpolateData(FromData, *this);
}
