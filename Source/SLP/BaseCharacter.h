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
	void Move(float AxisValue);
	void Strafe(float AxisValue);
	void LockOn();
	void LookUp(float AxisValue);
	void LookRight(float AxisValue);

	TArray<struct FHitResult> OutHits;

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
	float LockOnRange = 500;

	UPROPERTY(EditAnywhere)
	float SweepRadius = 500;

	void HandleCamera(float DeltaTime);
	void DoTrace();

	int32 ClosestEnemy = 0;
};
