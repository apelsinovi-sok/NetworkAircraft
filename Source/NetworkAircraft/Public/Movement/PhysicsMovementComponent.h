// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FDeferredForcesLoader.h"
#include "GameFramework/PawnMovementComponent.h"

#include "PhysicsMovementComponent.generated.h"

class UNetworkPhysicsComponent;
/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class NETWORKAIRCRAFT_API UPhysicsMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core Physics Movement|Debug")
    bool bDebugingMessages = false;

    virtual void BeginPlay() override;

    virtual void OnCreatePhysicsState() override;

    virtual void OnDestroyPhysicsState() override;

    virtual bool ShouldCreatePhysicsState() const override;

    void ErrorMessage(FString message, FColor color, double seconds);

    /*Hook into the OnCreatePhysicsState*/
    virtual void InitializeNetworkPhysicsMovement() {}

    /*Hook into the OnCreatePhysicsState*/
    virtual void SimulatePhysicsTick(double DeltaTime, Chaos::FRigidBodyHandle_Internal* Handle){};

	USkeletalMeshComponent* GetSkeletalMesh() const;

	UStaticMeshComponent* GetStaticMesh() const;

	UMeshComponent* GetMesh() const;

public:
	
	UPhysicsMovementComponent(const FObjectInitializer& ObjectInitializer);
	
    virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

    virtual void AsyncPhysicsTickComponent(float DeltaTime, float SimTime) override;

    bool IsUsingNetworkPhysicsPrediction() const { return bUsingNetworkPhysicsPrediction; }

    UNetworkPhysicsComponent* GetNetworkPhysicsComponent() const{ return NetworkPhysicsComponent; }

	FBodyInstance* GetBodyInstance() const
    {
	    return BodyInstance;
    }

    UFUNCTION(BlueprintCallable)
    void AddForce(const FVector& Force, bool IsAccelerationChange);

    UFUNCTION(BlueprintCallable)
    void AddForceAtPosition(const FVector& Force, const FVector& Position);

    UFUNCTION(BlueprintCallable)
    void AddTorqueInRadians(const FVector& TorqueInRadians, bool IsAccelerationChange);

    UFUNCTION(BlueprintCallable)
    void AddImpulse(const FVector& Impulse, bool IsVelocityChange);

    UFUNCTION(BlueprintCallable)
    void AddImpulseAtPosition(const FVector& Impulse, const FVector& Position);

    UFUNCTION(BlueprintCallable)
    void ApplyForces();

    UFUNCTION(BlueprintPure)
    bool IsApplyingForces() const;

private:

    UPROPERTY()
    TObjectPtr<UNetworkPhysicsComponent> NetworkPhysicsComponent = nullptr;

    Chaos::FRigidBodyHandle_Internal* RigidHandle = nullptr;
	FBodyInstance* BodyInstance = nullptr;

	FDeferredForcesLoader DeferredForcesLoader;

	bool bUsingNetworkPhysicsPrediction = false;
	
};
