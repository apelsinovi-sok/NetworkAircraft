// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVehicleMovementTraits.h"
#include "Movement/PhysicsMovementComponent.h"
#include "Simulation/ControlImputConfigurartion.h"
#include "Simulation/SimulationConfigurations.h"

#include "VehicleSimulationComponent.generated.h"

/**
 * 
 */

UENUM(Blueprintable)
enum EThrottleInputAxis : uint8
{
	TIA_Throttle_X    UMETA(DisplayName = "Throttle Input on X Axis"),
	TIA_Throttle_Y    UMETA(DisplayName = "Throttle Input on Y Axis"),
	TIA_Throttle_Z    UMETA(DisplayName = "Throttle Input on Z Axis"),
};


UCLASS(meta = (BlueprintSpawnableComponent))
class NETWORKAIRCRAFT_API UVehicleSimulationComponent : public UPhysicsMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVehicleSimulationInputs SimulationInputs;
    
	UPROPERTY()
	FVehicleSimulationState SimulationState;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core Vehicle|Simulation")
	TArray<FPhysicsStaticThrusterSetup> StaticThrusterConfiguration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FPhysicsSimpleRotationConfiguration SimpleRotationControl;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FThrottleConfiguration ThrottleAxisX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FThrottleConfiguration ThrottleAxisY;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FThrottleConfiguration ThrottleAxisZ;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FInputRateConfiguration PitchInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FInputRateConfiguration RollInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Core VehicleInput")
	FInputRateConfiguration YawInput;

	virtual void BeginPlay() override;

	virtual void InitializeNetworkPhysicsMovement() override;
	
	/*Hook into the OnCreatePhysicsState*/
	virtual void SimulatePhysicsTick(double DeltaTime, Chaos::FRigidBodyHandle_Internal* Handle) override;

	void CreateThrusterSetups();
    
    FVector GetBoneOrSocketLocation(FName BoneOrSocketName) const;
    
    void UpdateStateGT(double deltaTime);
    
    void ClearRawInputs();
    
    void ApplyInputs(double deltaTime);
    
    void UpdateSimulation(double deltaTime, Chaos::FRigidBodyHandle_Internal* Handle);
    
    void ApplyThrustForces(double deltaTime, Chaos::FRigidBodyHandle_Internal* Handle);
	
	void ApplyTorque(double deltaTime, Chaos::FRigidBodyHandle_Internal* Handle);
	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetThrottleInput(double value, EThrottleInputAxis axis);

	UFUNCTION(BlueprintCallable)
	void SetYawInput(double value);

	UFUNCTION(BlueprintCallable)
	void SetPitchInput(double value);

	UFUNCTION(BlueprintCallable)
	void SetRollInput(double value);

	UFUNCTION(BlueprintCallable)
	void ResetSimulation();

	UFUNCTION(BlueprintPure)
	FORCEINLINE FVehicleSimulationState GetCurrentVehicleState() const { return SimulationState; }

	UFUNCTION(BlueprintCallable)
	void LoadCurrentVehicleState(FVehicleSimulationState value);

	
private:
	double RawThrottleInputX = 0.f;
	double RawThrottleInputY = 0.f;
	double RawThrottleInputZ = 0.7f;
	double RawPitchInput = 0.f;
	double RawRollInput = 0.f;
	double RawYawInput = 0.f;
};



