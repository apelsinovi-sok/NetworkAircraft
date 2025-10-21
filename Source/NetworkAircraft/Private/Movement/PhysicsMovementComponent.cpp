// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/PhysicsMovementComponent.h"

#include "Physics/NetworkPhysicsComponent.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

UPhysicsMovementComponent::UPhysicsMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bUsingNetworkPhysicsPrediction = Chaos::FPhysicsSolverBase::IsNetworkPhysicsPredictionEnabled();

	if (bUsingNetworkPhysicsPrediction)
	{
		static const FName NetworkPhysicsComponentName(TEXT("PC_NetworkPhysicsComponent"));
		NetworkPhysicsComponent = CreateDefaultSubobject<UNetworkPhysicsComponent, UNetworkPhysicsComponent>(NetworkPhysicsComponentName);
		NetworkPhysicsComponent->SetNetAddressable(); // Make DSO components net addressable
		NetworkPhysicsComponent->SetIsReplicated(true);
	}
}

void UPhysicsMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPhysicsMovementComponent::OnCreatePhysicsState()
{
	Super::OnCreatePhysicsState();

	UWorld* World = GetWorld();
	if (World->IsGameWorld())
	{
		if (UpdatedComponent && UpdatedPrimitive)
		{
			SetAsyncPhysicsTickEnabled(true);
		}
		
		InitializeNetworkPhysicsMovement();
	}
	
	if (UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(UpdatedComponent))
	{
		BodyInstance = Mesh->GetBodyInstance();
	}
}

void UPhysicsMovementComponent::OnDestroyPhysicsState()
{
	Super::OnDestroyPhysicsState();
	
	if (UpdatedComponent)
	{
		UpdatedComponent->RecreatePhysicsState();
	}
	
	if (NetworkPhysicsComponent)
	{
		NetworkPhysicsComponent->RemoveDataHistory();
	}
}

bool UPhysicsMovementComponent::ShouldCreatePhysicsState() const
{
	if (!IsRegistered() || IsBeingDestroyed())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World->IsGameWorld())
	{
		FPhysScene* PhysScene = World->GetPhysicsScene();
		if (PhysScene && UpdatedComponent && UpdatedPrimitive)
		{
			return true;
		}
	}
	return false;
}

void UPhysicsMovementComponent::ErrorMessage(FString message, FColor color, double seconds)
{
	if (bDebugingMessages)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(- 1,seconds, color, message, true);
		}
	}
}


USkeletalMeshComponent* UPhysicsMovementComponent::GetSkeletalMesh() const
{
	return Cast<USkeletalMeshComponent>(GetPawnOwner()->GetRootComponent());
}

UStaticMeshComponent* UPhysicsMovementComponent::GetStaticMesh() const
{
	return Cast<UStaticMeshComponent>(GetPawnOwner()->GetRootComponent());
}

UMeshComponent* UPhysicsMovementComponent::GetMesh() const
{
	return Cast<UMeshComponent>(GetPawnOwner()->GetRootComponent());
}

void UPhysicsMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	UNavMovementComponent::SetUpdatedComponent(NewUpdatedComponent);

	PawnOwner = NewUpdatedComponent ? Cast<APawn>(NewUpdatedComponent->GetOwner()) : nullptr;
}

void UPhysicsMovementComponent::AsyncPhysicsTickComponent(float DeltaTime, float SimTime)
{
	Super::AsyncPhysicsTickComponent(DeltaTime, SimTime);
	
	if (!BodyInstance) { return; }
	
	if (const auto Handle = BodyInstance->ActorHandle)
	{
		RigidHandle = Handle->GetPhysicsThreadAPI();
	}

	UWorld* World = GetWorld();
	if (World && RigidHandle)
	{
		SimulatePhysicsTick(DeltaTime, RigidHandle);
		DeferredForcesLoader.Apply(RigidHandle);
	}
}

void UPhysicsMovementComponent::AddForce(const FVector& Force, bool IsAccelerationChange)
{
	DeferredForcesLoader.Add(FAddForceData(Force, IsAccelerationChange));
}

void UPhysicsMovementComponent::AddForceAtPosition(const FVector& Force, const FVector& Position)
{
	DeferredForcesLoader.Add(FAddForceAtPositionData(Force, Position));
}

void UPhysicsMovementComponent::AddTorqueInRadians(const FVector& TorqueInRadians, bool IsAccelerationChange)
{
	DeferredForcesLoader.Add(FAddTorqueData(TorqueInRadians, IsAccelerationChange));
}

void UPhysicsMovementComponent::AddImpulse(const FVector& Impulse, bool IsVelocityChange)
{
	DeferredForcesLoader.Add(FAddImpulseData(Impulse, IsVelocityChange));
}

void UPhysicsMovementComponent::AddImpulseAtPosition(const FVector& Impulse, const FVector& Position)
{
	DeferredForcesLoader.Add(FAddImpulseAtPositionData(Impulse, Position));
}

void UPhysicsMovementComponent::ApplyForces()
{
	DeferredForcesLoader.UnLock();
}

bool UPhysicsMovementComponent::IsApplyingForces() const
{
	return DeferredForcesLoader.IsLocked();
}
