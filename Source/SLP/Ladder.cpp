// Fill out your copyright notice in the Description page of Project Settings.


#include "Ladder.h"

#include "Components/BoxComponent.h"
#include "BaseCharacter.h"
// Sets default values
ALadder::ALadder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LadderDownCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LadderDownCollision"));
	LadderDownCollision -> SetupAttachment(RootComponent);

	LadderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LadderMesh"));
	LadderMesh -> SetupAttachment(LadderDownCollision);

	LadderUpCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LadderUpCollision"));
	LadderUpCollision -> SetupAttachment(LadderDownCollision);
}

// Called when the game starts or when spawned
void ALadder::BeginPlay()
{
	Super::BeginPlay();
	
	LadderHeight = LadderUpCollision -> GetComponentLocation().Z - LadderDownCollision -> GetComponentLocation().Z;
	UE_LOG(LogTemp, Warning, TEXT("Ladder Height: %f"), LadderHeight);
}

// Called every frame
void ALadder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DetectPlayer();
}

bool ALadder::DetectPlayer()
{
	TArray<AActor*> OverlappingActorsDown;
	TArray<AActor*> OverlappingActorsUp;
	LadderDownCollision -> GetOverlappingActors(OverlappingActorsDown, ABaseCharacter::StaticClass());
	LadderUpCollision -> GetOverlappingActors(OverlappingActorsUp, ABaseCharacter::StaticClass());
	if(OverlappingActorsDown.Num() > 0)
	{
		for(AActor* Actor : OverlappingActorsDown)
		{
			if(Actor -> ActorHasTag("Player"))
			{
				UE_LOG(LogTemp, Warning, TEXT("(Down) Player in range!"));
				return true;
			} 
		}
	}
	else if(OverlappingActorsUp.Num() > 0)
	{
		for(AActor* Actor : OverlappingActorsUp)
		{
			if(Actor -> ActorHasTag("Player"))
			{
				UE_LOG(LogTemp, Warning, TEXT("(Up) Player in range!"));
				return true;
			} 
		}
	}
	return false;
}

