// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class SLP_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
    class UInputMappingContext * InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputStrafe;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputLookUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputLookRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputLockOn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputCameraRightLockedOn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputCameraLeftLockedOn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction * InputRunDashRoll;

	void Move(const struct FInputActionValue & Value);
	void Strafe(const struct FInputActionValue & Value);
	void LookUp(const struct FInputActionValue & Value);
	void LookRight(const struct FInputActionValue & Value);
	void DetermineCameraPlacement(const struct FInputActionValue & Value);
	void Sprint(const struct FInputActionValue & Value);
	void Roll(const struct FInputActionValue & Value);

	bool bCameraOnTheRightLockedOn;

	void LockOn();
	void ToggleEnemyWhenLockedOn(float AxisValue);
	void ChangeCameraPositionWhenLockedOn(float DeltaTime);

	TArray<AActor*> NearestActors;

	UPROPERTY()
	class APlayerController* PlayerController;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	float MoveAxisValue;
	float StrafeAxisValue;
	bool bIsLockedOn;
	
	UPROPERTY(EditAnywhere)
	float LockOnRange = 1000;

	UPROPERTY(EditAnywhere)
	float SweepRadius = 300;

	void HandleCamera(float DeltaTime);
	void DoTrace();

	int32 ClosestEnemy = 0;
	bool bIsPlayerRunning = false;
	UPROPERTY(EditAnywhere)
	float RunSpeed = 20.f;
};
