// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/ControlImputConfigurartion.h"

double FInputRateConfiguration::GetInterpilatedValue(double DeltaTime, double CurrentValue, double Target)
{
	if (bEnable)
	{
		const double dist = Target - CurrentValue;
		
        const bool bRising = ((dist > 0.0f) == (CurrentValue > 0.0f)) || ((dist != 0.f) && (CurrentValue == 0.f));
		
        if (bRising)
        {
        	return FMath::FInterpConstantTo(CurrentValue, Target, DeltaTime, RiseRate);
        }
       
		return FMath::FInterpConstantTo(CurrentValue, Target, DeltaTime, FallRate);
	}

	return Target;
}

double FThrottleControlConfiguration::GetThrottleValue(double value, double deltaSeconds)\
{
	if (!bEnableThrottleControl)
	{
		return CurrentThrottleValue = value;
	}
	
	double min = 0.f;
	

	if (bCanReverseThrottle) { min = -1.f; }
	
	if (value > 0.0f)
	{
		double target = FMath::Clamp(CurrentThrottleValue + RiseRate, min, 1.f);
		CurrentThrottleValue = FMath::FInterpTo(CurrentThrottleValue, target, deltaSeconds, InterpRise);
	}
	else if (value < 0.0f)
	{
		double target = FMath::Clamp(CurrentThrottleValue - FallRate, min, 1.f);
		CurrentThrottleValue = FMath::FInterpTo(CurrentThrottleValue, target, deltaSeconds, InterpFall);
	}
	
	return CurrentThrottleValue;
}

double FThrottleConfiguration::GetInterpilatedValue(double deltaTime, double CurrentValue, double RawValue)
{
	double adjRaw = ThrottleControl.GetThrottleValue(RawValue, deltaTime);
	
	return ThrottleInputRate.GetInterpilatedValue(deltaTime, CurrentValue, adjRaw);
}

void FPhysicsDampingConfiguration::AddDampingEffect(const FVector worldVelocity, FVector& outForce)
{
	if (bEnabled)
	{
		FVector damping = worldVelocity * Damping;
		outForce -= damping;
	}
}

FVector FInputScalingConfiguration::GetTorque(const double controlInput, const FVector& Axis)
{
	if (bEnabled)
	{
		return Axis * controlInput * Scaling;
	}

	return FVector::ZeroVector;
}

