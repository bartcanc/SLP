// Fill out your copyright notice in the Description page of Project Settings.


#include "Elevator.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BaseCharacter.h"

// Sets default values
AElevator::AElevator()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrentState = ElevatorState::Down;
	PreviousState = ElevatorState::Down;

	ElevatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElevatorMesh"));
	RootComponent = ElevatorMesh;

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh -> SetupAttachment(ElevatorMesh);

	ElevatorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ElevatorTrigger"));
	ElevatorTrigger -> SetupAttachment(TriggerMesh);
	
	bIsElevatorTriggered = false;
}

// Called when the game starts or when spawned
void AElevator::BeginPlay()
{
	Super::BeginPlay();
	
	// UE_LOG(LogTemp, Display, TEXT("start location: %s"), *StartLocation.ToString());
	// UE_LOG(LogTemp, Display, TEXT("end location: %s"), *EndLocation.ToString());
	
	switch (ElevatorStartPosition)
	{
		case true:
			EndLocation = GetActorLocation();		
			StartLocation = EndLocation + FVector(0, 0, -MoveDistance); 
			CurrentState = ElevatorState::Up;
			PreviousState = ElevatorState::Up;
			break;
		case false:
			StartLocation = GetActorLocation();
			EndLocation = StartLocation + FVector(0, 0, MoveDistance); 
			CurrentState = ElevatorState::Down;
			PreviousState = ElevatorState::Down;
			break;
	};
}

// Called every frame
void AElevator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(DetectPlayer() and !bIsElevatorTriggered)
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer set! Activating elevator..."));
		GetWorld() -> GetTimerManager().SetTimer(ActivationTimerHandle, this, &AElevator::MovePlatform, 1.0f, false);
		bIsElevatorTriggered = true;
	}
	MovePlatform();
}

bool AElevator::DetectPlayer()
{
	TArray<AActor*> OverlappingActors;
	ElevatorTrigger -> GetOverlappingActors(OverlappingActors, ABaseCharacter::StaticClass());
	if(OverlappingActors.Num() > 0)
	{
		for(AActor* Actor : OverlappingActors)
		{
			if(Actor -> ActorHasTag("Player"))
			{
				//UE_LOG(LogTemp, Warning, TEXT("Player detected!"));
				return true;
			} 
		}
	}
	return false;
}

void AElevator::MovePlatform()
{
	if(GetWorld()->GetTimerManager().IsTimerActive(ActivationTimerHandle))
	{
		return;
	}
	
	switch (CurrentState)
	{
		case ElevatorState::Down:
		if(bIsElevatorTriggered)
		{
			if(PreviousState == ElevatorState::MovingDown)	break;	// if the player stays on the elevator, it won't trigger again
			GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &AElevator::OnElevatorMoveFinished, MoveDuration, false);
			CurrentState = ElevatorState::MovingUp;
			PreviousState = ElevatorState::Down;
		}
		else
		{
			PreviousState = CurrentState;
		}
		break;
		case ElevatorState::Up:
		if(bIsElevatorTriggered)
		{
			if(PreviousState == ElevatorState::MovingUp)	break;	// if the player stays on the elevator, it won't trigger again
			GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &AElevator::OnElevatorMoveFinished, MoveDuration, false);
			CurrentState = ElevatorState::MovingDown;
			PreviousState = ElevatorState::Up;
		}
		else
		{
			PreviousState = CurrentState;
		}
		break;
		case ElevatorState::MovingUp:
		{
			float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MoveTimerHandle);
			float Alpha = ElapsedTime / MoveDuration;
			SetActorLocation(FMath::Lerp(StartLocation, EndLocation, Alpha));
			break;
		}
		case ElevatorState::MovingDown:
		{
			UE_LOG(LogTemp, Warning, TEXT("moving down state"));
			float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MoveTimerHandle);
			float Alpha = ElapsedTime / MoveDuration;
			SetActorLocation(FMath::Lerp(EndLocation, StartLocation, Alpha));
			break;
		}
		default:
			break;
	}
}

void AElevator::OnElevatorMoveFinished()
{
	if (CurrentState == ElevatorState::MovingUp)
	{
		CurrentState = ElevatorState::Up;
		PreviousState = ElevatorState::MovingUp;
		bIsElevatorTriggered = false;
	}
	else if (CurrentState == ElevatorState::MovingDown)
	{
		CurrentState = ElevatorState::Down;
		PreviousState = ElevatorState::MovingDown;
		bIsElevatorTriggered = false;
	}
}