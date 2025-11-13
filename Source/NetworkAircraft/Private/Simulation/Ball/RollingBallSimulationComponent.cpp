#include "Simulation/Ball/RollingBallSimulationComponent.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

URollingBallSimulationComponent::URollingBallSimulationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	static const FName NetworkPhysicsComponentName(TEXT("PC_NetworkPhysicsComponent"));
	NetworkPhysicsComponent = CreateDefaultSubobject<UNetworkPhysicsComponent, UNetworkPhysicsComponent>(NetworkPhysicsComponentName);
	NetworkPhysicsComponent->SetNetAddressable();
	NetworkPhysicsComponent->SetIsReplicated(true);
}

void URollingBallSimulationComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	UNavMovementComponent::SetUpdatedComponent(NewUpdatedComponent);
	PawnOwner = NewUpdatedComponent ? Cast<APawn>(NewUpdatedComponent->GetOwner()) : nullptr;
}

bool URollingBallSimulationComponent::ShouldCreatePhysicsState() const
{
	if (!IsRegistered() || IsBeingDestroyed()) return false;

	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		FPhysScene* PhysScene = World->GetPhysicsScene();
		if (PhysScene && UpdatedComponent && UpdatedPrimitive) return true;
	}
	return false;
}

void URollingBallSimulationComponent::OnCreatePhysicsState()
{
	Super::OnCreatePhysicsState();

	if (UWorld* World = GetWorld())
	{
		if (World->IsGameWorld())
		{
			InitializeBall();
			if (NetworkPhysicsComponent)
			{
				NetworkPhysicsComponent->CreateDataHistory<FRollingBallPhysicsMovementTraits>(this);
			}
		}
	}

	if (UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(UpdatedComponent))
	{
		BodyInstance = Mesh->GetBodyInstance();
	}
}

void URollingBallSimulationComponent::OnDestroyPhysicsState()
{
	Super::OnDestroyPhysicsState();
	if (UpdatedComponent) UpdatedComponent->RecreatePhysicsState();
	if (NetworkPhysicsComponent) NetworkPhysicsComponent->RemoveDataHistory();
}

void URollingBallSimulationComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateTargetAltitude(DeltaTime);
}

void URollingBallSimulationComponent::InitializeBall()
{
	if (UpdatedComponent && UpdatedPrimitive)
	{
		SetAsyncPhysicsTickEnabled(true);
	}
}

void URollingBallSimulationComponent::AsyncPhysicsTickComponent(float DeltaTime, float SimTime)
{
	Super::AsyncPhysicsTickComponent(DeltaTime, SimTime);

	if (!BodyInstance) return;

	if (const auto Handle = BodyInstance->ActorHandle)
	{
		RigidHandle = Handle->GetPhysicsThreadAPI();
	}

	if (!RigidHandle) return;

	// Текущее физическое состояние
	const FTransform WorldTM = FTransform(RigidHandle->R(), RigidHandle->X());
	BallWorldTransform = WorldTM;
	BallState.BallForwardAxis = WorldTM.GetUnitAxis(EAxis::X);
	BallState.BallRightAxis = WorldTM.GetUnitAxis(EAxis::Y);
	BallState.BallUpAxis = WorldTM.GetUnitAxis(EAxis::Z);
	BallUpAxis = BallWorldTransform.GetUnitAxis(EAxis::Z);

	BallState.WorldVelocity = RigidHandle->V();

	CurrentAltitude = WorldTM.GetLocation().Z;
	BallState.CurrentAltitude = CurrentAltitude;

	// Jump как событие
	if (BallInputs.JumpCount != PreviousJumpCount)
	{
		RigidHandle->SetLinearImpulse(FVector(0,0,500.f), true);
	}

	const FVector DesiredForwardVector = BallInputs.TravelDirection.Vector();
	

	if (BallInputs.PidEnableActiveStatus != PreviousPidEnableActiveStatus)
	{
		PreviousPidEnableActiveStatusBool = !PreviousPidEnableActiveStatusBool;
	}

	if (PreviousPidEnableActiveStatusBool)
	{
		float NewPidThrottle = GetPIDThrottle(DeltaTime);
		PidThrottle = NewPidThrottle;
		// Небольшое сглаживание выхода для подавления высокочастотных колебаний
		// PidThrottle = FMath::Lerp(PidThrottle, NewPidThrottle, 0.05f);
	} else
	{
		PidThrottle = 0;
	}

	// PidThrottle = FMath::RoundToFloat(PidThrottle * 100.f) / 100.f;
	
	UE_LOG(LogTemp, Warning, TEXT("%f"), PidThrottle);

	// if (PidThrottle - 0.33 > KINDA_SMALL_NUMBER)
	// {
	// 	PidThrottle = 0.33;
	// }
		
 

	// Применяем силу вверх
	RigidHandle->AddForce(PidThrottle * BallState.BallUpAxis * 300000.f, true);
	// RigidHandle->AddForce(PidThrottle * FVector::UpVector * 300000.f, true);
	// RigidHandle->AddForce(0.3266 * BallState.BallUpAxis * 300000.f, true);
	// RigidHandle->AddForce(BallInputs.ThrottleInput * DesiredForwardVector * 80000.f, true);
	// RigidHandle->AddForce(BallInputs.ThrottleInput * BallState.BallForwardAxis * 80000.f, true);

	RigidHandle->AddTorque(BallInputs.SteeringInput * BallState.BallUpAxis * 160000.f, true);
	RigidHandle->AddTorque(BallInputs.ThrottleInput * BallState.BallRightAxis * 20000.f, false);

	BallInputs.SteeringInput = 0;
	BallInputs.ThrottleUp = 0.f;
	BallInputs.ThrottleInput = 0.f;
	
	// Записываем локальные PID-переменные в реплицируемый стейт, чтобы при rewind их можно было восстановить
	// BallState.ErrorSum = LocalErrorSum;
	// BallState.PidThrottle = PidThrottle;

	PreviousJumpCount = BallInputs.JumpCount;
	PreviousPidEnableActiveStatus = BallInputs.PidEnableActiveStatus;

	PreviousPidThrottle = PidThrottle;
}


// void URollingBallSimulationComponent::AsyncPhysicsTickComponent(float DeltaTime, float SimTime)
// {
// 	Super::AsyncPhysicsTickComponent(DeltaTime, SimTime);
//
// 	if (!BodyInstance) return;
//
// 	if (const auto Handle = BodyInstance->ActorHandle)
// 	{
// 		RigidHandle = Handle->GetPhysicsThreadAPI();
// 	}
//
// 	if (!RigidHandle) return;
//
// 	// Текущее физическое состояние
// 	const FTransform WorldTM = FTransform(RigidHandle->R(), RigidHandle->X());
// 	BallWorldTransform = WorldTM;
// 	BallState.BallForwardAxis = WorldTM.GetUnitAxis(EAxis::X);
// 	BallState.BallRightAxis = WorldTM.GetUnitAxis(EAxis::Y);
// 	BallState.BallUpAxis = WorldTM.GetUnitAxis(EAxis::Z);
// 	
// 	// === ИСПРАВЛЕНИЕ: Используем МИРОВУЮ вертикаль для PID ===
// 	// Вместо локальной оси Z объекта используем константную мировую ось
// 	// Это предотвращает изменение направления силы при повороте
// 	BallUpAxis = FVector::UpVector; // Всегда (0, 0, 1)
// 	
// 	BallState.WorldVelocity = RigidHandle->V();
// 	CurrentAltitude = WorldTM.GetLocation().Z;
// 	BallState.CurrentAltitude = CurrentAltitude;
//
// 	// Jump как событие
// 	if (BallInputs.JumpCount != PreviousJumpCount)
// 	{
// 		RigidHandle->SetLinearImpulse(FVector(0,0,500.f), true);
// 	}
//
// 	const FVector DesiredForwardVector = BallInputs.TravelDirection.Vector();
// 	
// 	// Получаем PID с улучшенной защитой от дрожания
// 	float NewPidThrottle = GetPIDThrottle(DeltaTime);
//
// 	// === ИСПРАВЛЕНИЕ: Адаптивное сглаживание ===
// 	// Более агрессивное сглаживание при малых изменениях (уменьшает дрожание)
// 	// Менее агрессивное при больших изменениях (сохраняет отзывчивость)
// 	const float DeltaPid = FMath::Abs(NewPidThrottle - PidThrottle);
// 	const float SmoothingFactor = FMath::GetMappedRangeValueClamped(
// 		FVector2D(0.0f, 0.1f),  // Диапазон изменения
// 		FVector2D(0.1f, 0.5f),  // Диапазон коэффициента сглаживания
// 		DeltaPid
// 	);
// 	
// 	PidThrottle = FMath::Lerp(PidThrottle, NewPidThrottle, SmoothingFactor);
//
// 	// Обнуляем локальный ThrottleUp
// 	BallInputs.ThrottleUp = 0.f;
//
// 	// === ВАЖНО: Используем МИРОВУЮ вертикаль для вертикальной силы ===
// 	RigidHandle->AddForce(PidThrottle * FVector::UpVector * 300000.f, true);
// 	
// 	// Горизонтальное движение и поворот остаются без изменений
// 	RigidHandle->AddForce(BallInputs.ThrottleInput * DesiredForwardVector * 80000.f, true);
// 	RigidHandle->AddTorque(BallInputs.SteeringInput * BallUpAxis * 160000.f, true);
// 	BallInputs.SteeringInput = 0;
//
// 	// Записываем локальные PID-переменные в реплицируемый стейт
// 	BallState.ErrorSum = LocalErrorSum;
// 	BallState.PidThrottle = PidThrottle;
//
// 	PreviousJumpCount = BallInputs.JumpCount;
// }

// void URollingBallSimulationComponent::AsyncPhysicsTickComponent(float DeltaTime, float SimTime)
// {
// 	Super::AsyncPhysicsTickComponent(DeltaTime, SimTime);
//
// 	if (!BodyInstance) return;
//
// 	if (const auto Handle = BodyInstance->ActorHandle)
// 	{
// 		RigidHandle = Handle->GetPhysicsThreadAPI();
// 	}
//
// 	if (!RigidHandle) return;
//
// 	// Текущее физическое состояние
// 	const FTransform WorldTM = FTransform(RigidHandle->R(), RigidHandle->X());
// 	BallWorldTransform = WorldTM;
// 	BallState.BallForwardAxis = WorldTM.GetUnitAxis(EAxis::X);
// 	BallState.BallRightAxis = WorldTM.GetUnitAxis(EAxis::Y);
// 	BallState.BallUpAxis = WorldTM.GetUnitAxis(EAxis::Z);
// 	BallUpAxis = BallWorldTransform.GetUnitAxis(EAxis::Z);
//
// 	BallState.WorldVelocity = RigidHandle->V();
//
// 	CurrentAltitude = WorldTM.GetLocation().Z;
// 	BallState.CurrentAltitude = CurrentAltitude;
//
// 	// Jump как событие
// 	if (BallInputs.JumpCount != PreviousJumpCount)
// 	{
// 		RigidHandle->SetLinearImpulse(FVector(0,0,500.f), true);
// 	}
//
// 	const FVector DesiredForwardVector = BallInputs.TravelDirection.Vector();
// 	
// 	// Получаем PID с deadband защитой
// 	float NewPidThrottle = GetPIDThrottle(DeltaTime);
//
// 	// Небольшое сглаживание выхода для подавления высокочастотных колебаний
// 	PidThrottle = FMath::Lerp(PidThrottle, NewPidThrottle, 0.3f);
//
// 	// Обнуляем локальный ThrottleUp
// 	BallInputs.ThrottleUp = 0.f;
//
// 	// === КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Применяем силу только если она значимая ===
// 	// Это предотвращает "шум" в физике при зависании
// 	const float ForceThreshold = 0.01f; // 1% от максимума
// 	if (FMath::Abs(PidThrottle) > ForceThreshold)
// 	{
// 		RigidHandle->AddForce(PidThrottle * BallState.BallUpAxis * 300000.f, true);
// 	}
// 	// Если PidThrottle близок к нулю - вообще не применяем вертикальную силу
// 	// Объект естественно зависнет на текущей высоте без микрофлуктуаций
//
// 	// Горизонтальное движение и поворот применяются ВСЕГДА (независимо от PID)
// 	RigidHandle->AddTorque(BallInputs.SteeringInput * BallUpAxis * 160000.f, true);
// 	RigidHandle->AddForce(BallInputs.ThrottleInput * DesiredForwardVector * 80000.f, true);
// 	BallInputs.SteeringInput = 0;
//
// 	// Записываем локальные PID-переменные в реплицируемый стейт
// 	BallState.ErrorSum = LocalErrorSum;
// 	BallState.PidThrottle = PidThrottle;
//
// 	PreviousJumpCount = BallInputs.JumpCount;
// }

float URollingBallSimulationComponent::GetPIDThrottle(float DeltaTime)
{
	// Используем текущую целевую высоту из инпута
	const float TargetAltitude = BallInputs.TargetAltitude;

	const float Error = TargetAltitude - CurrentAltitude;

	const float ErrorMeters = Error / 100.0f;
	const float VelocityMetersPerSec = BallState.WorldVelocity.Z / 100.0f;

	// UE_LOG(LogTemp, Warning, TEXT("%f"), ErrorMeters);

	if (FMath::Abs(ErrorMeters) < 1.f)
	{
		return 0.326;
	}
	
	// Предварительно накапливаем интеграл
	LocalErrorSum += ErrorMeters * DeltaTime;
	LocalErrorSum = FMath::Clamp(LocalErrorSum, -20.0f, 20.0f);

	// PID коэффициенты — можно тонко подбирать
	const float kp = 0.1f;
	const float ki = 0.02f;
	const float kd = 0.08f;
	
	// const float MovementFactor = FMath::Clamp(FMath::Abs(VelocityMetersPerSec) * 10.f, 0.f, 1.f);
	// const float kp = FMath::Lerp(0.02f, 0.1f, MovementFactor);
	// const float kd = FMath::Lerp(0.05f, 0.15f, MovementFactor);

	const float P_term = kp * ErrorMeters;
	const float I_term = ki * LocalErrorSum;
	const float D_term = kd * (-VelocityMetersPerSec);

	float result = P_term + I_term + D_term;
	
	// Возвращаем в пределах [-1,1]
	result = FMath::Clamp(result, -1.f, 1.f);

	// LastPidOutput = result;
	return result;
	// return 0.326;
}


void URollingBallSimulationComponent::UpdateTargetAltitude(float DeltaTime)
{
	if (FMath::Abs(BallInputs.ThrottleUp) > KINDA_SMALL_NUMBER)
	{
		const float AltitudeChangeSpeed = 200.0f;
		BallInputs.TargetAltitude += BallInputs.ThrottleUp * AltitudeChangeSpeed * DeltaTime;
		BallInputs.TargetAltitude = FMath::Clamp(BallInputs.TargetAltitude, 0.0f, 10000.0f);
		BallInputs.AltitudeVersion++;
	}
}

void URollingBallSimulationComponent::SetThrottleInput(float InThrottle)
{
	const float FinalThrottle = FMath::Clamp(InThrottle, -1.f, 1.f);
	BallInputs.ThrottleInput = InThrottle;
}

void URollingBallSimulationComponent::SetSteeringInput(float InSteering)
{
	const float FinalSteering = FMath::Clamp(InSteering, -1.f, 1.f);
	BallInputs.SteeringInput = InSteering;
}

void URollingBallSimulationComponent::SetTravelDirectionInput(FRotator InTravelDirection)
{
	BallInputs.TravelDirection = InTravelDirection;
}

void URollingBallSimulationComponent::Jump()
{
	BallInputs.JumpCount++;
}

void URollingBallSimulationComponent::SetThrottleUp(float ThrottleUp)
{
	BallInputs.ThrottleUp = ThrottleUp;
}

void URollingBallSimulationComponent::SwitchPidStatus()
{
	BallInputs.PidEnableActiveStatus++;
}

bool FNetworkBallInputs::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FNetworkPhysicsData::SerializeFrames(Ar);

	Ar << BallInputs.SteeringInput;
	Ar << BallInputs.ThrottleInput;
	Ar << BallInputs.TravelDirection;
	Ar << BallInputs.JumpCount;
	Ar << BallInputs.ThrottleUp;
	Ar << BallInputs.TargetAltitude;
	Ar << BallInputs.AltitudeVersion;
	Ar << BallInputs.PidEnableActiveStatus;

	bOutSuccess = true;
	return bOutSuccess;
}

void FNetworkBallInputs::ApplyData(UActorComponent* NetworkComponent) const
{
	if (URollingBallSimulationComponent* BallMover = Cast<URollingBallSimulationComponent>(NetworkComponent))
	{
		BallMover->BallInputs = BallInputs;
	}
}

void FNetworkBallInputs::BuildData(const UActorComponent* NetworkComponent)
{
	if (const URollingBallSimulationComponent* BallMover = Cast<const URollingBallSimulationComponent>(NetworkComponent))
	{
		BallInputs = BallMover->BallInputs;
	}
}

void FNetworkBallInputs::InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData)
{
	const FNetworkBallInputs& MinInput = static_cast<const FNetworkBallInputs&>(MinData);
	const FNetworkBallInputs& MaxInput = static_cast<const FNetworkBallInputs&>(MaxData);

	const float LerpFactor = MaxInput.LocalFrame == LocalFrame
		? 1.0f / (MaxInput.LocalFrame - MinInput.LocalFrame + 1)
		: (LocalFrame - MinInput.LocalFrame) / (MaxInput.LocalFrame - MinInput.LocalFrame);

	BallInputs.ThrottleInput = FMath::Lerp(MinInput.BallInputs.ThrottleInput, MaxInput.BallInputs.ThrottleInput, LerpFactor);
	BallInputs.SteeringInput = FMath::Lerp(MinInput.BallInputs.SteeringInput, MaxInput.BallInputs.SteeringInput, LerpFactor);
	BallInputs.TravelDirection = FMath::Lerp(MinInput.BallInputs.TravelDirection, MaxInput.BallInputs.TravelDirection, LerpFactor);
	BallInputs.ThrottleUp = FMath::Lerp(MinInput.BallInputs.ThrottleUp, MaxInput.BallInputs.ThrottleUp, LerpFactor);
	BallInputs.TargetAltitude = FMath::Lerp(MinInput.BallInputs.TargetAltitude, MaxInput.BallInputs.TargetAltitude, LerpFactor);

	BallInputs.JumpCount = LerpFactor < 0.5f ? MinInput.BallInputs.JumpCount : MaxInput.BallInputs.JumpCount;
	BallInputs.PidEnableActiveStatus = LerpFactor < 0.5f ? MinInput.BallInputs.PidEnableActiveStatus : MaxInput.BallInputs.PidEnableActiveStatus;
}

void FNetworkBallInputs::MergeData(const FNetworkPhysicsData& FromData)
{
	InterpolateData(FromData, *this);
}

// ----------------- States --------------------------------

bool FNetworkBallStates::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FNetworkPhysicsData::SerializeFrames(Ar);

	Ar << BallState.BallForwardAxis;
	Ar << BallState.BallUpAxis;
	Ar << BallState.WorldVelocity;

	// добавлены поля для восстановления PID
	// Ar << BallState.CurrentAltitude;
	// Ar << BallState.ErrorSum;
	// Ar << BallState.PidThrottle;

	bOutSuccess = true;
	return bOutSuccess;
}

void FNetworkBallStates::ApplyData(UActorComponent* NetworkComponent) const
{
	if (URollingBallSimulationComponent* BallMover = Cast<URollingBallSimulationComponent>(NetworkComponent))
	{
		// Восстанавливаем состояние (важно — в таком порядке: сначала стейт, затем локальные PID-переменные)
		BallMover->BallState = BallState;

		// Восстанавливаем локальные переменные, используемые в PID
		BallMover->CurrentAltitude = BallState.CurrentAltitude;
		BallMover->LocalErrorSum = BallState.ErrorSum;
		BallMover->PidThrottle = BallState.PidThrottle;
	}
}

void FNetworkBallStates::BuildData(const UActorComponent* NetworkComponent)
{
	if (const URollingBallSimulationComponent* BallMover = Cast<const URollingBallSimulationComponent>(NetworkComponent))
	{
		BallState = BallMover->BallState;
	}
}

void FNetworkBallStates::InterpolateData(const FNetworkPhysicsData& MinData, const FNetworkPhysicsData& MaxData)
{
	const FNetworkBallStates& MinInput = static_cast<const FNetworkBallStates&>(MinData);
	const FNetworkBallStates& MaxInput = static_cast<const FNetworkBallStates&>(MaxData);

	const float LerpFactor = MaxInput.LocalFrame == LocalFrame
		? 1.0f / (MaxInput.LocalFrame - MinInput.LocalFrame + 1)
		: (LocalFrame - MinInput.LocalFrame) / (MaxInput.LocalFrame - MinInput.LocalFrame);

	BallState.BallForwardAxis = FMath::Lerp(MinInput.BallState.BallForwardAxis, MaxInput.BallState.BallForwardAxis, LerpFactor);
	BallState.BallUpAxis = FMath::Lerp(MinInput.BallState.BallUpAxis, MaxInput.BallState.BallUpAxis, LerpFactor);
	BallState.WorldVelocity = FMath::Lerp(MinInput.BallState.WorldVelocity, MaxInput.BallState.WorldVelocity, LerpFactor);

	BallState.CurrentAltitude = FMath::Lerp(MinInput.BallState.CurrentAltitude, MaxInput.BallState.CurrentAltitude, LerpFactor);
	BallState.ErrorSum = FMath::Lerp(MinInput.BallState.ErrorSum, MaxInput.BallState.ErrorSum, LerpFactor);
	BallState.PidThrottle = FMath::Lerp(MinInput.BallState.PidThrottle, MaxInput.BallState.PidThrottle, LerpFactor);
}

void FNetworkBallStates::MergeData(const FNetworkPhysicsData& FromData)
{
	InterpolateData(FromData, *this);
}
