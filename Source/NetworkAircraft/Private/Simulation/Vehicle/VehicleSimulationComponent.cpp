// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/Vehicle/VehicleSimulationComponent.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

#include "Simulation/SimulationConfigurations.h"


void UVehicleSimulationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVehicleSimulationComponent::InitializeNetworkPhysicsMovement()
{
	if (IsUsingNetworkPhysicsPrediction())
	{
		if (GetNetworkPhysicsComponent())
		{
			GetNetworkPhysicsComponent()->CreateDataHistory<FVehicleMovementTraits>(this);
		}
	}
}

void UVehicleSimulationComponent::SimulatePhysicsTick(double DeltaTime, Chaos::FRigidBodyHandle_Internal* Handle)
{
	if (Handle)
	{
		SimulationState.UpdateStatePT(Handle, GetGravityZ(), DeltaTime);
		
        Super::SimulatePhysicsTick(DeltaTime, Handle);
           
        ErrorMessage(FString::FromInt(SimulationState.Forward.Speed), FColor::Red, 0.0f);
        ErrorMessage(FString::FromInt(SimulationState.Right.Speed), FColor::Green, 0.0f);
        ErrorMessage(FString::FromInt(SimulationState.Up.Speed), FColor::Blue, 0.0f);
           
        ApplyInputs(DeltaTime);
		
        UpdateSimulation(DeltaTime, Handle);
		
        ApplyForces();
	}
}

void UVehicleSimulationComponent::CreateThrusterSetups()
{
	for (FPhysicsStaticThrusterSetup& t : StaticThrusterConfiguration)
	{
		if (!t.ThrustPoint.BoneOrSocketName.IsNone())
		{
			t.ThrustPoint.SetForceLocation(GetBoneOrSocketLocation(t.ThrustPoint.BoneOrSocketName));
		}
	}
}

FVector UVehicleSimulationComponent::GetBoneOrSocketLocation(FName BoneOrSocketName) const
{
	if (USkeletalMeshComponent* mesh = GetSkeletalMesh())
	{
		if (mesh->GetSocketByName(BoneOrSocketName))
		{
			return mesh->GetSocketTransform(BoneOrSocketName, ERelativeTransformSpace::RTS_Component).GetLocation();
		}
	
		return mesh->GetBoneLocation(BoneOrSocketName, EBoneSpaces::ComponentSpace);
	}

	if (UStaticMeshComponent* mesh = GetStaticMesh())
	{
		if (mesh->GetSocketByName(BoneOrSocketName))
		{
			return mesh->GetSocketTransform(BoneOrSocketName, ERelativeTransformSpace::RTS_Component).GetLocation();
		}
	}

	return FVector::ZeroVector;
}

void UVehicleSimulationComponent::UpdateStateGT(double deltaTime)
{
	SimulationState.UpdateStateGT(GetBodyInstance(), GetGravityZ(), deltaTime);
	
	// SimulationInputs.ThrottleInput_XAxis = ThrottleAxisX.GetInterpilatedValue(deltaTime, SimulationInputs.ThrottleInput_XAxis, RawThrottleInputX, this);
	// SimulationInputs.ThrottleInput_YAxis = ThrottleAxisY.GetInterpilatedValue(deltaTime, SimulationInputs.ThrottleInput_YAxis, RawThrottleInputY, this);
	// SimulationInputs.ThrottleInput_ZAxis = ThrottleAxisZ.GetInterpilatedValue(deltaTime, SimulationInputs.ThrottleInput_ZAxis, RawThrottleInputZ, this);

	ThrottleAxisZ.State = this->GetCurrentVehicleState();
	
	// SimulationInputs.ThrottleInput_XAxis = ThrottleAxisX.GetInterpilatedValue(deltaTime, SimulationInputs.ThrottleInput_XAxis, SimulationInputs.RawThrottleInputX);
	// SimulationInputs.ThrottleInput_YAxis = ThrottleAxisY.GetInterpilatedValue(deltaTime, SimulationInputs.ThrottleInput_YAxis, SimulationInputs.RawThrottleInputY);
	SimulationInputs.ThrottleInput_ZAxis = ThrottleAxisZ.GetInterpilatedValue(deltaTime, SimulationInputs.RawThrottleInputZ);
	
	// if (GetOwner() && GetOwner()->HasAuthority())
	// {
	// 	UE_LOG(LogTemp, Log, TEXT("CurrentThrottleValue = %s, RawThrottleValue : %s, %s"), *FString::SanitizeFloat(SimulationInputs.ThrottleInput_ZAxis) , *FString::SanitizeFloat(SimulationInputs.ThrottleInput_ZAxis), TEXT("Server"));
	// } else
	// {
	// 	UE_LOG(LogTemp, Log, TEXT("CurrentThrottleValue = %s, RawThrottleValue : %s, %s"), *FString::SanitizeFloat(SimulationInputs.ThrottleInput_ZAxis) , *FString::SanitizeFloat(SimulationInputs.ThrottleInput_ZAxis), TEXT("Client"));
	// }
	
	// SimulationInputs.PitchInput = PitchInput.GetInterpilatedValue(deltaTime, SimulationInputs.PitchInput, RawPitchInput, this);
	// SimulationInputs.RollInput = RollInput.GetInterpilatedValue(deltaTime, SimulationInputs.RollInput, RawRollInput, this);
	// SimulationInputs.YawInput = YawInput.GetInterpilatedValue(deltaTime, SimulationInputs.YawInput, RawYawInput, this);

	YawInput.State = this->GetCurrentVehicleState();
	
	// SimulationInputs.PitchInput = PitchInput.GetInterpilatedValue(deltaTime, SimulationInputs.PitchInput, SimulationInputs.RawPitchInput);
	// SimulationInputs.RollInput = RollInput.GetInterpilatedValue(deltaTime, SimulationInputs.RollInput, SimulationInputs.RawRollInput);
	// SimulationInputs.YawInput = YawInput.GetInterpilatedValue(deltaTime, RawYawInput);
	SimulationInputs.YawInput = YawInput.GetYawValue(deltaTime, RawYawInput);
	
	SimulationState.CurrentThrottleValueX = ThrottleAxisX.ThrottleControl.GetCurrentThrottleValue();
	SimulationState.CurrentThrottleValueY = ThrottleAxisY.ThrottleControl.GetCurrentThrottleValue();
	SimulationState.CurrentThrottleValueZ = ThrottleAxisZ.ThrottleControl.GetCurrentThrottleValue();

	ClearRawInputs();
}

void UVehicleSimulationComponent::ClearRawInputs()
{
	// RawThrottleInputX = 0.f;
	// RawThrottleInputY = 0.f;
	// RawThrottleInputZ = 0.f;


	// TODO по неизветсной причине делает значение на сервере всегда нулевым
	SimulationInputs.RawThrottleInputX = 0.f;
	SimulationInputs.RawThrottleInputY = 0.f;
	SimulationInputs.RawThrottleInputZ = 0.f;
		
	// SimulationInputs.RawPitchInput = 0.f;
	// SimulationInputs.RawRollInput = 0.f;
	// SimulationInputs.RawYawInput = 0.f;
	
	// RawPitchInput = 0.f;
	// RawRollInput = 0.f;
	RawYawInput = 0.f;
}

void UVehicleSimulationComponent::ApplyInputs(double deltaTime)
{
	for (FPhysicsStaticThrusterSetup& thruster : StaticThrusterConfiguration)
	{
		switch (thruster.ControlInputSource)
		{
		case EInputControlType::ICT_Throttle_X:
			{
				thruster.SetThrottle(SimulationInputs.ThrottleInput_XAxis);
			}
			break;
		case EInputControlType::ICT_Throttle_Y:
			{
				thruster.SetThrottle(SimulationInputs.ThrottleInput_YAxis);
			}
			break;
		case EInputControlType::ICT_Throttle_Z:
			{
				thruster.SetThrottle(SimulationInputs.ThrottleInput_ZAxis);
			}
			break;
		case EInputControlType::ICT_Input_Pitch:
			{
				thruster.SetThrottle(SimulationInputs.PitchInput);
			}
			break;
		case EInputControlType::ICT_Input_Roll:
			{
				thruster.SetThrottle(SimulationInputs.RollInput);
			}
			break;
		case EInputControlType::ICT_Input_Yaw:
			{
				thruster.SetThrottle(SimulationInputs.YawInput);
			}
			break;
		}
	}
}

void UVehicleSimulationComponent::UpdateSimulation(double deltaTime, Chaos::FRigidBodyHandle_Internal* Handle)
{
	ApplyThrustForces(deltaTime, Handle);
	ApplyTorque(deltaTime, Handle);
}

void UVehicleSimulationComponent::ApplyThrustForces(double deltaTime, Chaos::FRigidBodyHandle_Internal* Handle)
{
	if (Handle)
	{
		for (FPhysicsStaticThrusterSetup& thruster : StaticThrusterConfiguration)
		{
			FVector CenterOfMass = Handle->CenterOfMass();
			CenterOfMass.Z = 0.0f;
			thruster.SetWorldAltitude(SimulationState.WorldTransform.GetTranslation().Z);
			
			const FVector WorldLocation = SimulationState.WorldTransform.TransformPosition(thruster.ThrustPoint.GetLocation() + CenterOfMass);
			const FQuat rotation = SimulationState.WorldTransform.GetRotation();
			FVector Force = rotation.RotateVector(thruster.GetForce());
			AddForceAtPosition(Force, WorldLocation);
		}
	}
}

void UVehicleSimulationComponent::ApplyTorque(double deltaTime, Chaos::FRigidBodyHandle_Internal* Handle)
{
	if (Handle)
	{
		FVector totalTorque = FVector::ZeroVector;
		
		if (SimpleRotationControl.bEnableRotationControl)
		{
			// totalTorque -= SimpleRotationControl.TorqueInputScaling.RollInput.GetTorque(SimulationInputs.RollInput, SimulationState.Forward.Axis);
			// totalTorque += SimpleRotationControl.TorqueInputScaling.PitchInput.GetTorque(SimulationInputs.PitchInput, SimulationState.Right.Axis);
			totalTorque += SimpleRotationControl.TorqueInputScaling.YawInput.GetTorque(SimulationInputs.YawInput, SimulationState.Up.Axis);
			// totalTorque += SimpleRotationControl.TorqueInputScaling.YawFromRollInput.GetTorque(SimulationInputs.RollInput, SimulationState.Up.Axis);
			
			// SimpleRotationControl.TorqueInputScaling.TorqueDamping.AddDampingEffect(SimulationState.WorldAngularVelocity, totalTorque);
		}
		
		AddTorqueInRadians(totalTorque, true);
	}
}

void UVehicleSimulationComponent::SetThrottleInput(double value, EThrottleInputAxis axis)
{
	switch (axis)
	{
	case TIA_Throttle_X:
		// RawThrottleInputX = FMath::Clamp(value, -1.f, 1.f);
			SimulationInputs.RawThrottleInputX =  FMath::Clamp(value, -1.f, 1.f);;
		break;
	case TIA_Throttle_Y:
		// RawThrottleInputY = FMath::Clamp(value, -1.f, 1.f);
			SimulationInputs.RawThrottleInputY =  FMath::Clamp(value, -1.f, 1.f);;
		break;
	case TIA_Throttle_Z:
		// RawThrottleInputZ = FMath::Clamp(value, -1.f, 1.f);
		SimulationInputs.RawThrottleInputZ = FMath::Clamp(value, -1.f, 1.f);
		break;
	default:
		ErrorMessage("SetThrottleInput: No Input to map to", FColor::Red, 2.f);
		break;
	}
}

void UVehicleSimulationComponent::SetYawInput(double value)
{
	RawYawInput = FMath::Clamp(value, -1.f, 1.f);
	// SimulationInputs.RawYawInput = FMath::Clamp(value, -1.f, 1.f);;
}

void UVehicleSimulationComponent::SetPitchInput(double value)
{
	// RawPitchInput = FMath::Clamp(value, -1.f, 1.f);
	SimulationInputs.RawPitchInput = FMath::Clamp(value, -1.f, 1.f);;
}

void UVehicleSimulationComponent::SetRollInput(double value)
{
	// RawRollInput = FMath::Clamp(value, -1.f, 1.f);
	SimulationInputs.RawRollInput = FMath::Clamp(value, -1.f, 1.f);;
}

void UVehicleSimulationComponent::ResetSimulation()
{
	ClearRawInputs();
	SimulationInputs.ResetInputs();
}

void UVehicleSimulationComponent::LoadCurrentVehicleState(FVehicleSimulationState value)
{
	ThrottleAxisX.ThrottleControl.SetCurrentThrottleValue(value.CurrentThrottleValueX);
	ThrottleAxisY.ThrottleControl.SetCurrentThrottleValue(value.CurrentThrottleValueY);
	ThrottleAxisZ.ThrottleControl.SetCurrentThrottleValue(value.CurrentThrottleValueZ);
}

void UVehicleSimulationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateStateGT(DeltaTime);
}