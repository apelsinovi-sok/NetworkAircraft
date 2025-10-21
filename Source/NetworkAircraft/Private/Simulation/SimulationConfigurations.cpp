// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/SimulationConfigurations.h"


FVector FForceDirectionConfiguration::GetThrustDirection()
{
	FVector Direction = FVector::ZeroVector;
	
	if (ThrustAxis == EForceControlAxis::FCA_Static_X)
	{
		Direction = FVector::UnitX();
	}
	if (ThrustAxis == EForceControlAxis::FCA_Static_Y)
	{
		Direction = FVector::UnitY();
	}
	if (ThrustAxis == EForceControlAxis::FCA_Static_Z)
	{
		Direction = FVector::UnitZ();
	}

	const FQuat rotationOffset = FRotator(PitchAngleOffset, YawAngleOffset, RollAngleOffset).Quaternion();
	Direction = rotationOffset.RotateVector(Direction);

	if (IsDirectionNegated)
	{
		Direction = Direction * -1;
	}

	return Direction;
}

double FPhysicsThrustForce::GetMaxThrust(double altitude)
{
	if (ThrustCurve)
	{
		return ThrustConditionPercent * (ThrustCurve->GetFloatValue(altitude) * MaxThrustMultiplier);
	}

	return ThrustConditionPercent * (MaxThrust * MaxThrustMultiplier);
}

void FPhysicsStaticThrusterSetup::SetThrottle(float input)
{
	ThrustInput = FMath::Clamp(input, -1.f, 1.f);
}

const FVector FPhysicsStaticThrusterSetup::GetForce()
{
	if (!bThrusterEnabled) { return FVector::ZeroVector; }

	return ThrustDirection.GetThrustDirection() * (ThrustInput * ThrusterForces.GetMaxThrust(Altitude));
}

