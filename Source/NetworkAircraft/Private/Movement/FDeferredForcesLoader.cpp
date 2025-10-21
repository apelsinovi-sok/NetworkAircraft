// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/FDeferredForcesLoader.h"

#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

void FDeferredForcesLoader::Apply(Chaos::FRigidBodyHandle_Internal* RigidHandle)
{
	if (bIsLocked) { return; }
	
	for (const FAddForceData& d : ApplyForces)
	{
		AddForce(RigidHandle, d);
	}

	for (const FAddForceAtPositionData& d : ApplyForcesAtPosition)
	{
		AddForceAtPosition(RigidHandle, d);
	}

	for (const FAddTorqueData& d : ApplyTorques)
	{
		AddTorque(RigidHandle, d);
	}

	for (const FAddImpulseData& d : ApplyImpulses)
	{
		AddImpulse(RigidHandle, d);
	}

	for (const FAddImpulseAtPositionData& d : ApplyAddImpulsesAtPosition)
	{
		AddImpulseAtPosition(RigidHandle, d);
	}

	Reset();
	bIsLocked = true;
}

void FDeferredForcesLoader::Reset()
{
	ApplyForces.Empty();
	ApplyForcesAtPosition.Empty();
	ApplyTorques.Empty();
	ApplyImpulses.Empty();
	ApplyAddImpulsesAtPosition.Empty();
}

void FDeferredForcesLoader::UnLock()
{
	bIsLocked = false;
}

bool FDeferredForcesLoader::IsLocked() const
{
	return bIsLocked;
}

void FDeferredForcesLoader::AddForce(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddForceData& Data)
{
	if (ensure(RigidHandle))
	{
		if (Data.bIsAccelerationChange)
		{
			const Chaos::FVec3 force = Data.Force * RigidHandle->M(); // F = A*M
			RigidHandle->AddForce(force, true);
		}
		else
		{
			RigidHandle->AddForce(Data.Force, true);
		}
	}
}

void FDeferredForcesLoader::AddForceAtPosition(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddForceAtPositionData& Data)
{
	if (ensure(RigidHandle))
	{
		const Chaos::FVec3 WorldCenterOfMass = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RigidHandle);
		const Chaos::FVec3 WorldTorque = Chaos::FVec3::CrossProduct(Data.Position - WorldCenterOfMass, Data.Force);
		RigidHandle->AddForce(Data.Force, true); 
		RigidHandle->AddTorque(WorldTorque, true);
	}
}

void FDeferredForcesLoader::AddTorque(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddTorqueData& Data)
{
	if (ensure(RigidHandle))
	{
		if (Data.bIsAccelerationChange)
		{
			RigidHandle->AddTorque(Chaos::FParticleUtilitiesGT::GetWorldInertia(RigidHandle) * Data.Torque, true);
		}
		else
		{
			RigidHandle->AddTorque(Data.Torque, true);
		}
	}
}

void FDeferredForcesLoader::AddImpulse(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddImpulseData& Data)
{
	if (ensure(RigidHandle))
	{
		if (Data.bIsVelocityChange)
		{
			const Chaos::FVec3 force = Data.Impulse * RigidHandle->M();
			RigidHandle->SetLinearImpulse(RigidHandle->LinearImpulse() + force, true);
		}
		else
		{
			RigidHandle->SetLinearImpulse(RigidHandle->LinearImpulse() + Data.Impulse, true);
		}
	}
}

void FDeferredForcesLoader::AddImpulseAtPosition(Chaos::FRigidBodyHandle_Internal* RigidHandle, const FAddImpulseAtPositionData& Data)
{
	if (ensure(RigidHandle))
	{
		const Chaos::FVec3 WorldCenterOfMass = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RigidHandle);
		const Chaos::FVec3 AngularImpulse = Chaos::FVec3::CrossProduct(Data.Position - WorldCenterOfMass, Data.Impulse);
		RigidHandle->SetLinearImpulse(RigidHandle->LinearImpulse() + Data.Impulse, true);
		RigidHandle->SetAngularImpulse(RigidHandle->AngularImpulse() + AngularImpulse, true);
	}
}




