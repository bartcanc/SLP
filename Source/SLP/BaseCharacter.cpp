// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "BaseAIController.h"
#include "Engine/World.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent -> SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm -> SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera -> SetupAttachment(SpringArm);

	MoveAxisValue = 0.0f;
	StrafeAxisValue = 0.0f;	

	bIsLockedOn = false;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());
	if(!PlayerController) return;
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleCamera(DeltaTime);
	if(bIsLockedOn)
	{
		ChangeCameraPositionWhenLockedOn(DeltaTime);
	}
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent -> BindAxis(TEXT("Move"), this, &ABaseCharacter::Move);
	PlayerInputComponent -> BindAxis(TEXT("Strafe"), this, &ABaseCharacter::Strafe);
	PlayerInputComponent -> BindAxis(TEXT("LookUp"), this, &ABaseCharacter::LookUp);
	PlayerInputComponent -> BindAxis(TEXT("LookRight"), this, &ABaseCharacter::LookRight);
	PlayerInputComponent -> BindAction(TEXT("LockOn"), EInputEvent::IE_Pressed, this, &ABaseCharacter::LockOn);
}

void ABaseCharacter::HandleCamera(float DeltaTime)
{
	// TODO: player can't lock on when the enemy is obstructed by a wall
	if (bIsLockedOn)
    {
		FVector FocusPoint = NearestActors[ClosestEnemy] -> GetActorLocation();
		FVector CameraLocation = Camera -> GetComponentLocation();

        FVector DirectionVector = FocusPoint - CameraLocation;
		if(DirectionVector.Length() > LockOnRange) 
		{
			bIsLockedOn = false;
			NearestActors.Empty();
			SpringArm -> SetRelativeLocation(FVector(0,0,60));
			return;
		}
        FRotator TargetCameraRotation = DirectionVector.Rotation();

		FRotator CurrentControlRotation = PlayerController -> GetControlRotation();
		FRotator NewControlRotation = FMath::RInterpTo(CurrentControlRotation, TargetCameraRotation, DeltaTime, 10.0f);

		PlayerController -> SetControlRotation(NewControlRotation);
		SetActorRotation(FRotator(0, NewControlRotation.Yaw, 0));		// player facing the lock on point at all times
		// TODO: when running, don't make player face the lock on point at all times
    }
	else
	{
		FVector Velocity = GetVelocity();
		if (!GetCharacterMovement() -> IsFalling() and Velocity.SizeSquared() > 0.0f)
		{
			FRotator TargetRotation = FRotationMatrix::MakeFromX(Velocity).Rotator();		// target rotation from velocity vector
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 20.0f);	// interpolation
			SetActorRotation(NewRotation);
		}
	}
}

void ABaseCharacter::DoTrace()
{
	TArray<FHitResult> OutHits;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + Camera -> GetForwardVector() * LockOnRange;

	FCollisionShape SweepSphere = FCollisionShape::MakeSphere(SweepRadius);

	if(GetWorld() -> UWorld::SweepMultiByChannel(
		OutHits, 
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel1,
		SweepSphere,
		Params
	))
	{
		UE_LOG(LogTemp, Warning, TEXT("Trace Successful!"));
		for (FHitResult& Hit : OutHits)
		{
			if (Hit.GetActor())
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *Hit.GetActor()->GetName());
				// DrawDebugSphere(
				// 	GetWorld(),
				// 	Hit.Location,
				// 	10.0f,
				// 	12,
				// 	FColor::Green,
				// 	false,
				// 	1.0f
				// );
				NearestActors.AddUnique(Hit.GetActor());
			}
		}
	}
}

void ABaseCharacter::ToggleEnemyWhenLockedOn(float AxisValue)
{
	if(AxisValue > 1)
	{
		if(!NearestActors.IsEmpty() and ClosestEnemy < NearestActors.Num()-1) ClosestEnemy++;
	}
	else if(AxisValue < -1)
	{
		if(!NearestActors.IsEmpty() and ClosestEnemy > 0) ClosestEnemy--;	
	}
}

void ABaseCharacter::ChangeCameraPositionWhenLockedOn(float DeltaTime)	// !!! needs to be tweaked
{
	UE_LOG(LogTemp, Warning, TEXT("RightVector: %s"), *GetActorRightVector().ToString());
	if(
		(GetActorRightVector().Y > 0 and GetActorRightVector().X > 0) or 
		(GetActorRightVector().Y < 0 and GetActorRightVector().X < 0) 
	)	// when moving to the right when locked on, change camera position to the right (5 sec)
	{		// right
		FVector TLocation = FMath::VInterpTo(SpringArm -> GetRelativeLocation(), FVector(0,50,60), DeltaTime, 5.f);
		SpringArm -> SetRelativeLocation(TLocation);
	}
	else if(
		(GetActorRightVector().Y < 0 and GetActorRightVector().X > 0) or 
		(GetActorRightVector().Y > 0 and GetActorRightVector().X < 0)
	)	// whem moving to the left, change camera position to the left
	{		// left
		FVector TLocation = FMath::VInterpTo(SpringArm -> GetRelativeLocation(), FVector(0,-50,60), DeltaTime, 5.f);
		SpringArm -> SetRelativeLocation(TLocation);
	}
}

void ABaseCharacter::Move(float AxisValue)
{
	MoveAxisValue = -AxisValue;

	if (PlayerController)
	{
		const FRotator Rotation = PlayerController -> GetControlRotation();		// controller rotation
		const FRotator YawRotation(0, Rotation.Yaw, 0);						// extracting yaw
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	// extracting direction

		AddMovementInput(Direction, AxisValue);
	}
}

void ABaseCharacter::Strafe(float AxisValue)
{
	StrafeAxisValue = -AxisValue;
	
	if (PlayerController)
	{
		const FRotator Rotation = PlayerController -> GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, AxisValue);
	}
}

void ABaseCharacter::LockOn()
{
	UE_LOG(LogTemp, Display, TEXT("MOUSE3 PRESSED"));
	DoTrace();

	// was the trace successful?
	if(!NearestActors.IsEmpty())
	{
		if(bIsLockedOn)	// if already locked on
		{				// lock off and clear the array
			UE_LOG(LogTemp, Warning, TEXT("Locked off..."));
			bIsLockedOn = false;
			NearestActors.Empty();
			SpringArm -> SetRelativeLocation(FVector(0,0,60));
		}
		else
		{				// lock on
			UE_LOG(LogTemp, Warning, TEXT("Locked on!"));
			bIsLockedOn = true;
			SpringArm -> SetRelativeLocation(FVector(0,50,60));
		}
		ClosestEnemy = 0;
	}
	else{
		// TODO: Reset camera to default position (when pressing MOUSE3 if not locked on)
	}
}

void ABaseCharacter::LookUp(float AxisValue)
{
	if(!bIsLockedOn) AddControllerPitchInput(AxisValue);
}

void ABaseCharacter::LookRight(float AxisValue)
{
	UE_LOG(LogTemp, Warning, TEXT("OutHits.Num(): %i"), NearestActors.Num()-1);
	UE_LOG(LogTemp, Warning, TEXT("ClosestEnemy: %i"), ClosestEnemy);
	if(!bIsLockedOn) 
	{
		AddControllerYawInput(AxisValue);
	}
	else
	{
		ToggleEnemyWhenLockedOn(AxisValue);
	}
}
