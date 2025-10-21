// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/Object/ObjectSimulationComponent.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

void UObjectSimulationComponent::InitializeNetworkPhysicsMovement()
{
	if (IsUsingNetworkPhysicsPrediction())
	{
		if (GetNetworkPhysicsComponent())
		{
			GetNetworkPhysicsComponent()->CreateDataHistory<FObjectPhysicsMovementTraits>(this);
		}
	}
}

void UObjectSimulationComponent::SimulatePhysicsTick(double DeltaTime, Chaos::FRigidBodyHandle_Internal* Handle)
{
	SimulationState.TravelDirection = FRotator(Handle->R());
	
	const FVector DesiredForwardVector = SimulationState.TravelDirection.Vector();
	const FVector DesiredRightVector = FRotationMatrix(SimulationState.TravelDirection).GetScaledAxis(EAxis::Y);
	const FVector DesiredUpVector = FRotationMatrix(SimulationState.TravelDirection).GetScaledAxis(EAxis::Z);

	if (SimulationInputs.BoostCount != SimulationState.PreviousJumpCount)
	{
		AddImpulse(DesiredUpVector * 500.f, false);
	}

	SimulationState.PreviousJumpCount = SimulationInputs.BoostCount;

	AddForce(SimulationInputs.ThrottleInput_XAxis * DesiredForwardVector * 800.f, true);
	AddForce(SimulationInputs.ThrottleInput_YAxis * DesiredRightVector * 800.f, true);

	AddTorqueInRadians(SimulationInputs.YawInput * FVector::UpVector * .5f, true);

	ApplyForces();

	SimulationInputs.ResetInputs();
}

void UObjectSimulationComponent::SetThrottleInputAxisX(double value)
{
	SimulationInputs.ThrottleInput_XAxis = FMath::Clamp(value, -1.f, 1.f);
}

void UObjectSimulationComponent::SetThrottleInputAxisY(double value)
{
	SimulationInputs.ThrottleInput_YAxis = FMath::Clamp(value, -1.f, 1.f);
}

void UObjectSimulationComponent::SetYawInput(double value)
{
	SimulationInputs.YawInput = FMath::Clamp(value, -1.f, 1.f);
}

void UObjectSimulationComponent::ImpulseBoost()
{
	SimulationInputs.BoostCount++;
}