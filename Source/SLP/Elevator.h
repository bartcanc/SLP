// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Elevator.generated.h"

enum class ElevatorState
{
	Down,
	Up,
	MovingUp,
	MovingDown
};

UCLASS()
class SLP_API AElevator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AElevator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* ElevatorMesh;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* TriggerMesh;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* ElevatorTrigger;

	ElevatorState CurrentState;
	ElevatorState PreviousState;

	bool DetectPlayer();
	void MovePlatform();
	void OnElevatorMoveFinished();
	FTimerHandle MoveTimerHandle;
	FTimerHandle ActivationTimerHandle;

	UPROPERTY(EditAnywhere)
	bool ElevatorStartPosition = false;	// up (true) or down (false)

	UPROPERTY(EditAnywhere)
	float MoveDistance = 1000.f;

	UPROPERTY(EditAnywhere)
	float MoveDuration = 5.f;

	FVector StartLocation;
	FVector EndLocation;
};
