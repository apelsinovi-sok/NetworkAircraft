// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/Vehicle/FVehicleMovementTraits.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Simulation/Vehicle/VehicleSimulationComponent.h"


void FVehicleSimulationInputs::ResetInputs()
{
	ThrottleInput_XAxis = 0.f;
	ThrottleInput_YAxis = 0.f;
	ThrottleInput_ZAxis = 0.f;
	
	YawInput = 0.f;
	PitchInput = 0.f;
	RollInput = 0.f;
	
	RawThrottleInputX = 0.f;
	RawThrottleInputY = 0.f;
	RawThrottleInputZ = 0.f;
	
	RawPitchInput = 0.f;
	RawYawInput = 0.f;
	RawRollInput = 0.f;
}

void FDirectionState::Update(FVector WorldVelocity, FVector WorldTransformAxis, double LocalAcceleration)
{
	Axis = WorldTransformAxis;
	Speed = FVector::DotProduct(WorldVelocity, WorldTransformAxis);
	Acceleration = LocalAcceleration;
}

void FVehicleSimulationState::UpdateStateGT(const FBodyInstance* TargetInstance, double GravityZ, double deltaTime)
{
	if (TargetInstance)
	{
		WorldTransform = TargetInstance->GetUnrealWorldTransform();
		WorldVelocity = TargetInstance->GetUnrealWorldVelocity();
		WorldAngularVelocity = TargetInstance->GetUnrealWorldAngularVelocityInRadians();
		WorldCOM = TargetInstance->GetCOMPosition();
		WorldVelocityNormal = WorldVelocity.GetSafeNormal();

		LocalVelocity = WorldTransform.InverseTransformVector(WorldVelocity);
		LocalAcceleration = (LocalVelocity - LastFrameVehicleLocalVelocity) / deltaTime;
		LastFrameVehicleLocalVelocity = LocalVelocity;
		LocalGForce = LocalAcceleration / FMath::Abs(GravityZ);

		Forward.Update(WorldVelocity, WorldTransform.GetUnitAxis(EAxis::X), LocalAcceleration.X);
		Right.Update(WorldVelocity, WorldTransform.GetUnitAxis(EAxis::Y), LocalAcceleration.Y);
		Up.Update(WorldVelocity, WorldTransform.GetUnitAxis(EAxis::Z), LocalAcceleration.Z);
	}
}

void FVehicleSimulationState::UpdateStatePT(const Chaos::FRigidBodyHandle_Internal* Handle, double GravityZ, double deltaTime)
{
	if (Handle)
	{
		const FTransform WorldITM(Handle->R(), Handle->X());

		WorldTransform = WorldITM;
		WorldVelocity = Handle->V();
		WorldAngularVelocity = Handle->W();
		WorldCOM = Handle->CenterOfMass();
		WorldVelocityNormal = WorldVelocity.GetSafeNormal();

		LocalVelocity = WorldTransform.InverseTransformVector(WorldVelocity);
		LocalAcceleration = (LocalVelocity - LastFrameVehicleLocalVelocity) / deltaTime;
		LastFrameVehicleLocalVelocity = LocalVelocity;
		LocalGForce = LocalAcceleration / FMath::Abs(GravityZ);

		Forward.Update(WorldVelocity, WorldTransform.GetUnitAxis(EAxis::X), LocalAcceleration.X);
		Right.Update(WorldVelocity, WorldTransform.GetUnitAxis(EAxis::Y), LocalAcceleration.Y);
		Up.Update(WorldVelocity, WorldTransform.GetUnitAxis(EAxis::Z), LocalAcceleration.Z);
	}
}

void FVehicleNetPhysMoveStates::ApplyData(UActorComponent* NetworkComponent) const
{
	if (UVehicleSimulationComponent* PhysicsMover = Cast<UVehicleSimulationComponent>(NetworkComponent))
	{
		PhysicsMover->SimulationState = MovementStates;
	}
}

void FVehicleNetPhysMoveStates::BuildData(const UActorComponent* NetworkComponent)
{
	if (NetworkComponent)
	{
		if (const UVehicleSimulationComponent* PhysicsMover = Cast<const UVehicleSimulationComponent>(NetworkComponent))
		{
			MovementStates = PhysicsMover->SimulationState;
		}
	}
}

void FVehicleNetPhysMoveStates::DecayData(float DecayAmount)
{
	DecayAmount = FMath::Min(DecayAmount * 2, 1.0f);
}

bool FVehicleNetPhysMoveStates::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	SerializeFrames(Ar);
	return true;
}

void FVehicleNetPhysMoveStates::InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData)
{
}

void FVehicleNetPhysMoveStates::MergeData(const FNetworkPhysicsData& FromData)
{
	InterpolateData(FromData, *this);
}



void FVehicleNetPhysMoveInputs::ApplyData(UActorComponent* NetworkComponent) const
{
	if (UVehicleSimulationComponent* PhysicsMover = Cast<UVehicleSimulationComponent>(NetworkComponent))
	{
		PhysicsMover->SimulationInputs = MovementInputs;
	}
}

void FVehicleNetPhysMoveInputs::BuildData(const UActorComponent* NetworkComponent)
{
	if (NetworkComponent)
	{
		if (const UVehicleSimulationComponent* PhysicsMover = Cast<const UVehicleSimulationComponent>(NetworkComponent))
		{
			MovementInputs = PhysicsMover->SimulationInputs;
		}
	}
}

void FVehicleNetPhysMoveInputs::DecayData(float DecayAmount)
{
	DecayAmount = FMath::Min(DecayAmount * 2, 1.0f);

	MovementInputs.ThrottleInput_XAxis = FMath::Lerp(MovementInputs.ThrottleInput_XAxis, 0.0f, DecayAmount);
	MovementInputs.ThrottleInput_YAxis = FMath::Lerp(MovementInputs.ThrottleInput_YAxis, 0.0f, DecayAmount);
	MovementInputs.ThrottleInput_ZAxis = FMath::Lerp(MovementInputs.ThrottleInput_ZAxis, 0.0f, DecayAmount);

	MovementInputs.RawThrottleInputX = FMath::Lerp(MovementInputs.RawThrottleInputX, 0.0f, DecayAmount);
	MovementInputs.RawThrottleInputY = FMath::Lerp(MovementInputs.RawThrottleInputY, 0.0f, DecayAmount);
	MovementInputs.RawThrottleInputZ = FMath::Lerp(MovementInputs.RawThrottleInputZ, 0.0f, DecayAmount);

	MovementInputs.YawInput = FMath::Lerp(MovementInputs.YawInput, 0.0f, DecayAmount);
	MovementInputs.PitchInput = FMath::Lerp(MovementInputs.PitchInput, 0.0f, DecayAmount);
	MovementInputs.RollInput = FMath::Lerp(MovementInputs.RollInput, 0.0f, DecayAmount);

	// MovementInputs.RawYawInput = FMath::Lerp(MovementInputs.RawYawInput, 0.0f, DecayAmount);
	// MovementInputs.RawPitchInput = FMath::Lerp(MovementInputs.RawPitchInput, 0.0f, DecayAmount);
	// MovementInputs.RawRollInput = FMath::Lerp(MovementInputs.RawRollInput, 0.0f, DecayAmount);
}

bool FVehicleNetPhysMoveInputs::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	SerializeFrames(Ar);
	Ar << MovementInputs.ThrottleInput_XAxis;
	Ar << MovementInputs.ThrottleInput_YAxis;
	Ar << MovementInputs.ThrottleInput_ZAxis;
	Ar << MovementInputs.YawInput;
	Ar << MovementInputs.PitchInput;
	Ar << MovementInputs.RollInput;
	Ar << MovementInputs.RawThrottleInputX;
	Ar << MovementInputs.RawThrottleInputY;
	Ar << MovementInputs.RawThrottleInputZ;
	// Ar << MovementInputs.RawYawInput;
	// Ar << MovementInputs.RawPitchInput;
	// Ar << MovementInputs.RawRollInput;
	
	bOutSuccess = true;
	return bOutSuccess;
}

void FVehicleNetPhysMoveInputs::InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData)
{
	const FVehicleNetPhysMoveInputs& MinInput = static_cast<const FVehicleNetPhysMoveInputs&>(MinData);
	const FVehicleNetPhysMoveInputs& MaxInput = static_cast<const FVehicleNetPhysMoveInputs&>(MaxData);

	const float LerpFactor = MaxInput.LocalFrame == LocalFrame
		? 1.0f / (MaxInput.LocalFrame - MinInput.LocalFrame + 1) // Merge from min into max
		: (LocalFrame - MinInput.LocalFrame) / (MaxInput.LocalFrame - MinInput.LocalFrame); // Interpolate from min to max

	MovementInputs.ThrottleInput_XAxis = FMath::Lerp(MinInput.MovementInputs.ThrottleInput_XAxis, MaxInput.MovementInputs.ThrottleInput_XAxis, LerpFactor);
	MovementInputs.ThrottleInput_YAxis = FMath::Lerp(MinInput.MovementInputs.ThrottleInput_YAxis, MaxInput.MovementInputs.ThrottleInput_YAxis, LerpFactor);
	MovementInputs.ThrottleInput_ZAxis = FMath::Lerp(MinInput.MovementInputs.ThrottleInput_ZAxis, MaxInput.MovementInputs.ThrottleInput_ZAxis, LerpFactor);
	
	MovementInputs.RawThrottleInputX = FMath::Lerp(MinInput.MovementInputs.RawThrottleInputX, MaxInput.MovementInputs.RawThrottleInputX, LerpFactor);
	MovementInputs.RawThrottleInputY = FMath::Lerp(MinInput.MovementInputs.RawThrottleInputY, MaxInput.MovementInputs.RawThrottleInputY, LerpFactor);
	MovementInputs.RawThrottleInputZ = FMath::Lerp(MinInput.MovementInputs.RawThrottleInputZ, MaxInput.MovementInputs.RawThrottleInputZ, LerpFactor);

	MovementInputs.YawInput = FMath::Lerp(MinInput.MovementInputs.YawInput, MaxInput.MovementInputs.YawInput, LerpFactor);
	MovementInputs.PitchInput = FMath::Lerp(MinInput.MovementInputs.PitchInput, MaxInput.MovementInputs.PitchInput, LerpFactor);
	MovementInputs.RollInput = FMath::Lerp(MinInput.MovementInputs.RollInput, MaxInput.MovementInputs.RollInput, LerpFactor);

	// MovementInputs.RawYawInput = FMath::Lerp(MinInput.MovementInputs.RawYawInput, MaxInput.MovementInputs.RawYawInput, LerpFactor);
	// MovementInputs.RawPitchInput = FMath::Lerp(MinInput.MovementInputs.RawPitchInput, MaxInput.MovementInputs.RawPitchInput, LerpFactor);
	// MovementInputs.RawRollInput = FMath::Lerp(MinInput.MovementInputs.RawRollInput, MaxInput.MovementInputs.RawRollInput, LerpFactor);
}

void FVehicleNetPhysMoveInputs::MergeData(const FNetworkPhysicsData& FromData)
{
	InterpolateData(FromData, *this);
}



