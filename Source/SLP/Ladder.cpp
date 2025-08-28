// Fill out your copyright notice in the Description page of Project Settings.


#include "Ladder.h"

#include "Components/BoxComponent.h"
#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	LadderUpEndCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LadderUpEndCollision"));
	LadderUpEndCollision -> SetupAttachment(LadderUpCollision);

	LadderDownEndCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LadderDownEndCollision"));
	LadderDownEndCollision -> SetupAttachment(LadderDownCollision);
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
	
	if(PlayerActor)	CheckEnds();
	else PlayerActor = DetectPlayer();
}

AActor* ALadder::DetectPlayer()
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
				ABaseCharacter* PlayerChar = Cast<ABaseCharacter>(Actor);
				if(PlayerChar -> GetCurrentState() == PlayerCurrentState::Ladder) 
				{
					PlayerChar -> GetCharacterMovement() -> Velocity = FVector::ZeroVector;
					PlayerChar -> SetActorLocation(LadderDownEndCollision -> GetComponentLocation() + FVector(0, 0, 130));
					FRotator NewRotation = (LadderMesh -> GetComponentLocation() - PlayerChar -> GetActorLocation()).Rotation();
					PlayerChar -> SetActorRotation(FRotator
						(0, 
						NewRotation.Yaw, 
						0));
					return Actor;
				}
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
				ABaseCharacter* PlayerChar = Cast<ABaseCharacter>(Actor);
				if(PlayerChar -> GetCurrentState() == PlayerCurrentState::Ladder) 
				{
					PlayerChar -> GetCharacterMovement() -> Velocity = FVector::ZeroVector;
					PlayerChar -> SetActorLocation(LadderUpEndCollision -> GetComponentLocation() + FVector(0,0,-130));
					FRotator NewRotation = (LadderMesh -> GetComponentLocation() - PlayerChar -> GetActorLocation()).Rotation();
					PlayerChar -> SetActorRotation(FRotator
						(0, 
						NewRotation.Yaw, 
						0));
					return Actor;
				} 
			} 
		}
	}
	return nullptr;
}

void ALadder::CheckEnds()
{
	TArray<AActor*> OverlappingActorsDownEnd;
	TArray<AActor*> OverlappingActorsUpEnd;
	LadderDownEndCollision -> GetOverlappingActors(OverlappingActorsDownEnd, ABaseCharacter::StaticClass());
	LadderUpEndCollision -> GetOverlappingActors(OverlappingActorsUpEnd, ABaseCharacter::StaticClass());
	if(OverlappingActorsDownEnd.Num() > 0)
	{
		for(AActor* Actor : OverlappingActorsDownEnd)
		{
			if(Actor -> ActorHasTag("Player"))
			{
				UE_LOG(LogTemp, Warning, TEXT("(DownEnd) Player in range!"));
				ABaseCharacter* PlayerChar = Cast<ABaseCharacter>(Actor);
				if(PlayerChar -> GetCurrentState() == PlayerCurrentState::Ladder) 
				{
					UE_LOG(LogTemp, Warning, TEXT("Teleporting player to the bottom of the ladder"));
					PlayerChar -> GetCharacterMovement() -> SetMovementMode(EMovementMode::MOVE_Walking);
					PlayerChar -> SetCurrentState(PlayerCurrentState::Default);
					PlayerChar -> SetActorLocation(LadderDownCollision -> GetComponentLocation());
					PlayerChar -> SetCanRoll();
					PlayerActor = nullptr;
				} 
			} 
		}
	}
	else if(OverlappingActorsUpEnd.Num() > 0)
	{
		for(AActor* Actor : OverlappingActorsUpEnd)
		{
			if(Actor -> ActorHasTag("Player"))
			{
				UE_LOG(LogTemp, Warning, TEXT("(UpEnd) Player in range!"));
				ABaseCharacter* PlayerChar = Cast<ABaseCharacter>(Actor);
				if(PlayerChar -> GetCurrentState() == PlayerCurrentState::Ladder) 
				{
					UE_LOG(LogTemp, Warning, TEXT("Teleporting player to the bottom of the ladder"));
					PlayerChar -> GetCharacterMovement() -> SetMovementMode(EMovementMode::MOVE_Walking);
					PlayerChar -> SetCurrentState(PlayerCurrentState::Default);
					PlayerChar -> SetActorLocation(LadderUpCollision -> GetComponentLocation());
					PlayerChar -> SetCanRoll();
					PlayerActor = nullptr;
				} 
			} 
		}
	}
}

