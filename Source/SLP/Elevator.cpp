// Fill out your copyright notice in the Description page of Project Settings.


#include "Elevator.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	// TODO: add a slight delay before the elevator starts moving after the player enters the trigger
	MovePlatform();
}

bool AElevator::DetectPlayer()
{
	TArray<AActor*> OverlappingActors;
	ElevatorTrigger -> GetOverlappingActors(OverlappingActors);
	if(OverlappingActors.Num() > 0)
	{
		for(AActor* Actor : OverlappingActors)
		{
			if(Actor -> ActorHasTag("Player"))
			{
				return true;
			} 
		}
	}
	return false;
}

void AElevator::MovePlatform()
{
	switch (CurrentState)
	{
		case ElevatorState::Down:
		if(DetectPlayer())
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
		if(DetectPlayer())
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
	}
	else if (CurrentState == ElevatorState::MovingDown)
	{
		CurrentState = ElevatorState::Down;
		PreviousState = ElevatorState::MovingDown;
	}
}