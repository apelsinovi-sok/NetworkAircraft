// Fill out your copyright notice in the Description page of Project Settings.


#include "Simulation/ControlImputConfigurartion.h"

#include "Simulation/Vehicle/VehicleSimulationComponent.h"

// double FInputRateConfiguration::GetInterpilatedValue(double DeltaTime, double CurrentValue, double Target)
// {
// 	if (bEnable)
// 	{
// 		const double dist = Target - CurrentValue;
// 		
//         const bool bRising = ((dist > 0.0f) == (CurrentValue > 0.0f)) || ((dist != 0.f) && (CurrentValue == 0.f));
// 		
//         if (bRising)
//         {
//         	return FMath::FInterpConstantTo(CurrentValue, Target, DeltaTime, RiseRate);
//         }
// 		
// 		return FMath::FInterpConstantTo(CurrentValue, Target, DeltaTime, FallRate);
// 	}
// 	
// 	return Target;
// }

double FInputRateConfiguration::GetInterpilatedValue(double DeltaTime, double RawValue)
{
	if (RawValue > 0.0f)
	{
		CurrentYawThrottleValue = FMath::FInterpConstantTo(CurrentYawThrottleValue, CurrentYawThrottleValue+1, DeltaTime, RiseRate);
	}
	if (RawValue <  0.0f)
	{
		CurrentYawThrottleValue = FMath::FInterpConstantTo(CurrentYawThrottleValue, CurrentYawThrottleValue-1, DeltaTime, FallRate);
	}
	if (RawValue == 0)
	{
		CurrentYawThrottleValue = FMath::FInterpConstantTo(CurrentYawThrottleValue, 0, DeltaTime, FallRate);
	}
	// if (RawValue != 0)
	// {
		UE_LOG(LogTemp, Warning, TEXT("Yaw: Cur=%.3f, Raw=%.3f"), CurrentYawThrottleValue, RawValue);
	// }
	
	return CurrentYawThrottleValue;
}

// double FInputRateConfiguration::GetYawValue(double DeltaTime, double RawInput)
// {
// 	float TargetValue = FMath::Clamp(RawInput, -1.0f, 1.0f);
//         
// 	// Выбираем скорость изменения
// 	float InterpSpeed;
//         
// 	if (FMath::Abs(TargetValue) > FMath::Abs(CurrentYawThrottleValue))
// 	{
// 		// Ускорение - игрок хочет крутиться быстрее
// 		InterpSpeed = 0.5f;
// 	}
// 	else
// 	{
// 		// Замедление - игрок отпустил кнопку или снижает интенсивность
// 		InterpSpeed = -0.5f;
// 	}
//         
// 	// Плавная интерполяция к целевому значению
// 	CurrentYawThrottleValue = FMath::FInterpTo(CurrentYawThrottleValue, TargetValue, DeltaTime, InterpSpeed);
//         
// 	return CurrentYawThrottleValue;
// }

// double FInputRateConfiguration::GetYawValue(double DeltaSeconds, double RawInput)
// {
// 	// Параметры управления
// 	const double MaxYawRate = 90.0;        // градусов в секунду
// 	const double YawAcceleration = 180.0;  // градусов в секунду^2
// 	const double YawDamping = 5.0;         // коэффициент демпфирования
//
// 	// Текущая угловая скорость (из состояния физики)
// 	double CurrentYawRate = State.WorldAngularVelocity.Z * (180.0 / PI); // в градусах/с
//
// 	// Целевой yaw rate на основе ввода
// 	double TargetYawRate = RawInput * MaxYawRate; // RawInput ∈ [-1,1]
//
// 	// Ошибка (насколько не совпадает текущая скорость с желаемой)
// 	double Error = TargetYawRate - CurrentYawRate;
//
// 	// Простая модель ускорения (как бы "газ по вращению")
// 	double YawAccel = FMath::Clamp(Error * YawDamping, -YawAcceleration, YawAcceleration);
//
// 	// Интегрируем — получаем новую скорость
// 	double NewYawRate = CurrentYawRate + YawAccel * DeltaSeconds;
//
// 	// Ограничиваем угловую скорость
// 	NewYawRate = FMath::Clamp(NewYawRate, -MaxYawRate, MaxYawRate);
//
// 	// --- вот ключевая часть ---
// 	// Нормализуем скорость обратно в диапазон [-1,1]
// 	double NormalizedInput = NewYawRate / MaxYawRate;
//
// 	// Дополнительно сглаживаем (по желанию)
// 	// NormalizedInput = FMath::Clamp(NormalizedInput, -1.0, 1.0);
//
// 	// UE_LOG(LogTemp, Log, TEXT("Yaw: Raw=%.2f, Cur=%.2f, New=%.2f -> Norm=%.2f"),
// 	//     RawInput, CurrentYawRate, NewYawRate, NormalizedInput);
//
// 	return NormalizedInput;
// }

double FInputRateConfiguration::GetYawValue(double DeltaSeconds, double RawInput)
{
	// Параметры отклика
	const double AccelRate = 2.0;   // скорость нарастания yaw (сек)
	const double DecelRate = 4.0;   // скорость отпускания yaw (сек)

	// Движение в сторону нажатия
	if (FMath::Abs(RawInput) > KINDA_SMALL_NUMBER)
	{
		CurrentYawThrottleValue = FMath::FInterpTo(CurrentYawThrottleValue, RawInput, DeltaSeconds, AccelRate);
	}
	else
	{
		// Плавный возврат к нулю
		CurrentYawThrottleValue = FMath::FInterpTo(CurrentYawThrottleValue, 0.0, DeltaSeconds, DecelRate);
	}

	// Ограничиваем диапазон
	CurrentYawThrottleValue = FMath::Clamp(CurrentYawThrottleValue, -1.0, 1.0);

	return CurrentYawThrottleValue;
}


double FThrottleControlConfiguration::GetThrottleValue(double value, double deltaSeconds, FVehicleSimulationState State)
{
	if (value > 0.0f)
	{
		TargetAltitude = FMath::FInterpTo(TargetAltitude, TargetAltitude+10, deltaSeconds, 500);
		// UE_LOG(LogTemp, Log, TEXT("%s"), *FString::SanitizeFloat(TargetAltitude));
	}
	if (value < 0.0f)
	{
		TargetAltitude = FMath::FInterpTo(TargetAltitude, TargetAltitude-10, deltaSeconds, 500);
		// UE_LOG(LogTemp, Log, TEXT("%s"), *FString::SanitizeFloat(TargetAltitude));
	}
	

	float CurrentAltitude = State.WorldTransform.GetLocation().Z;
	float Error = TargetAltitude - CurrentAltitude;

	// Переводим в метры для удобства
	float ErrorMeters = Error / 100.0f;
	float VelocityMetersPerSec = State.WorldVelocity.Z / 100.0f;

	ErrorSum += ErrorMeters * deltaSeconds;
	ErrorSum = FMath::Clamp(ErrorSum, -10.0f, 10.0f);

	float Derivative = -VelocityMetersPerSec;

	// Коэффициенты теперь более понятны (работаем с метрами)
	float kp = 0.1f;   
	float ki = 0.02f; 
	float kd = 0.15f; 

	float P_term = kp * ErrorMeters;
	float I_term = ki * ErrorSum;
	float D_term = kd * Derivative;

	float result = P_term + I_term + D_term;

	// Логи для отладки
	// UE_LOG(LogTemp, Warning, TEXT("Alt: %.0fcm, Target: %.0fcm, Err: %.2fm"), 
	// 	CurrentAltitude, TargetAltitude, ErrorMeters);
	// UE_LOG(LogTemp, Warning, TEXT("P: %.3f, I: %.3f, D: %.3f → %.3f"), 
	// 	P_term, I_term, D_term, result);

	CurrentThrottleValue = FMath::Clamp(result, -1.f, 1.f);
	
	return CurrentThrottleValue;
}

double FThrottleConfiguration::GetInterpilatedValue(double deltaTime, double RawValue)
{
 	double adjRaw = ThrottleControl.GetThrottleValue(RawValue, deltaTime, State);
	
	return adjRaw;
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

