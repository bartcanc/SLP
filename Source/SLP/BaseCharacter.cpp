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
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

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
	bCameraOnTheRightLockedOn = false;
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
	auto NPlayerController = Cast<APlayerController>(GetController());

    auto EISubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NPlayerController->GetLocalPlayer());
    EISubsystem -> AddMappingContext(InputMapping, 0);
	auto PlayerEIComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	PlayerEIComponent -> BindAction(InputMove, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
	PlayerEIComponent -> BindAction(InputStrafe, ETriggerEvent::Triggered, this, &ABaseCharacter::Strafe);
	PlayerEIComponent -> BindAction(InputLookUp, ETriggerEvent::Triggered, this, &ABaseCharacter::LookUp);
	PlayerEIComponent -> BindAction(InputLookRight, ETriggerEvent::Triggered, this, &ABaseCharacter::LookRight);
	PlayerEIComponent -> BindAction(InputLockOn, ETriggerEvent::Started, this, &ABaseCharacter::LockOn);

	PlayerEIComponent -> BindAction(InputCameraRightLockedOn, ETriggerEvent::Triggered, this, &ABaseCharacter::DetermineCameraPlacement);
	PlayerEIComponent -> BindAction(InputCameraLeftLockedOn, ETriggerEvent::Triggered, this, &ABaseCharacter::DetermineCameraPlacement);

	PlayerEIComponent -> BindAction(InputRunDashRoll, ETriggerEvent::Started, this, &ABaseCharacter::Roll);
	PlayerEIComponent -> BindAction(InputRunDashRoll, ETriggerEvent::Triggered, this, &ABaseCharacter::Sprint);

}

void ABaseCharacter::HandleCamera(float DeltaTime)
{
	// TODO: player can't lock on when the enemy is obstructed by a wall
	if (bIsLockedOn)
    {
		FVector FocusPoint = NearestActors[ClosestEnemy] -> GetActorLocation();
		FVector CameraLocation = Camera -> GetComponentLocation();

        FVector DirectionVector = FocusPoint - CameraLocation;
		if(DirectionVector.Length() > LockOnRange) 	// if the player leaves the lock on range
		{
			bIsLockedOn = false;		// leave the locked on state
			NearestActors.Empty();
			SpringArm -> SetRelativeLocation(FVector(0, 0, 60));
			return;
		}
        FRotator TargetCameraRotation = DirectionVector.Rotation();

		FRotator CurrentControlRotation = PlayerController -> GetControlRotation();
		FRotator NewControlRotation = FMath::RInterpTo(CurrentControlRotation, TargetCameraRotation, DeltaTime, 10.0f);

		PlayerController -> SetControlRotation(NewControlRotation);
		if(!bIsPlayerRunning) SetActorRotation(FRotator(0, NewControlRotation.Yaw, 0));		// player facing the lock on point at all times
		else
		{
			FRotator TargetRotation = FRotationMatrix::MakeFromX(GetVelocity()).Rotator();
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 20.0f);
			SetActorRotation(NewRotation);	
		}// TODO: when running, don't make player face the lock on point at all times
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
				//UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *Hit.GetActor()->GetName());
				NearestActors.AddUnique(Hit.GetActor());
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Nearest Actors Count: %d"), NearestActors.Num());
	}
}

void ABaseCharacter::ToggleEnemyWhenLockedOn(float AxisValue)
{
	if(!NearestActors.IsEmpty())
	{
		ClosestEnemy = abs(++ClosestEnemy) % NearestActors.Num();
	}
}

void ABaseCharacter::ChangeCameraPositionWhenLockedOn(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("RightVector: %s"), *GetActorRightVector().ToString());

	float Direction = 1;
	if(bCameraOnTheRightLockedOn)
	{		// right
		Direction = 1;
	}
	else{	// left
		Direction = -1;
	}
	FVector TLocation = FMath::VInterpTo(SpringArm -> GetRelativeLocation(), FVector(0, Direction*50, 60), DeltaTime, 5.f);
	SpringArm -> SetRelativeLocation(TLocation);
}

void ABaseCharacter::Move(const FInputActionValue & Value)	// refactored to use FInputActionValue
{
	UE_LOG(LogTemp, Display, TEXT("Move value: %f"), Value.Get<float>());
	MoveAxisValue = Value.Get<float>();

	if (PlayerController)
	{
		const FRotator Rotation = PlayerController -> GetControlRotation();		// controller rotation
		const FRotator YawRotation(0, Rotation.Yaw, 0);						// extracting yaw
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	// extracting direction

		if(bIsPlayerRunning) 
		{
			AddMovementInput(Direction, MoveAxisValue);
		}
		else 
		{
			AddMovementInput(Direction, 0.7 * MoveAxisValue);	// temporary solution, do research (speed increases when going diagonally))
		}
		
	}
}

void ABaseCharacter::Strafe(const FInputActionValue & Value)	// refactored to use FInputActionValue
{
	UE_LOG(LogTemp, Display, TEXT("Strafe value: %f"), Value.Get<float>());
	StrafeAxisValue = Value.Get<float>();
	
	if (PlayerController)
	{
		const FRotator Rotation = PlayerController -> GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if(bIsPlayerRunning) 
		{
			AddMovementInput(Direction, StrafeAxisValue);
		}
		else 
		{
			AddMovementInput(Direction, 0.7 * StrafeAxisValue);
		}
	}
}

void ABaseCharacter::LockOn()	// refactored to use FInputActionValue
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
			SpringArm -> SetRelativeLocation(FVector(0, 0, 60));
		}
		else
		{				// lock on
			UE_LOG(LogTemp, Warning, TEXT("Locked on!"));
			bIsLockedOn = true;
			SpringArm -> SetRelativeLocation(FVector(0, 50, 60));
		}
		ClosestEnemy = 0;
	}
	else{
		// TODO: Reset camera to default position (when pressing MOUSE3 if not locked on)
	}
}

void ABaseCharacter::LookUp(const FInputActionValue & Value)	// refactored to use FInputActionValue
{
	float AxisValue = Value.Get<float>();
	UE_LOG(LogTemp, Display, TEXT("Look Up value: %f"), AxisValue);
	if(!bIsLockedOn) AddControllerPitchInput(-AxisValue);
}

void ABaseCharacter::LookRight(const FInputActionValue & Value)	// refactored to use FInputActionValue
{
	float AxisValue = Value.Get<float>();
	//UE_LOG(LogTemp, Display, TEXT("Look Right value: %f"), AxisValue);
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

void ABaseCharacter::DetermineCameraPlacement(const FInputActionValue & Value)
{
	if(bIsLockedOn)
	{
		float AxisValue = Value.Get<float>();
		UE_LOG(LogTemp, Display, TEXT("Camera Placement value: %f"), AxisValue);
		if(AxisValue == 1)
		{
			bCameraOnTheRightLockedOn = false;
			UE_LOG(LogTemp, Warning, TEXT("Camera on the right locked on"));
		}
		else if(AxisValue == -1)
		{
			bCameraOnTheRightLockedOn = true;
			UE_LOG(LogTemp, Warning, TEXT("Camera on the left locked on"));
		}
	}
}

void ABaseCharacter::Sprint(const FInputActionValue & Value)
{

	//UE_LOG(LogTemp, Display, TEXT("Velocity value: %f"), GetVelocity().SizeSquared());
	if(GetVelocity().SizeSquared() > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("sprinting: %s"), Value.Get<bool>() ? TEXT("true") : TEXT("false"));
		bIsPlayerRunning = Value.Get<bool>();		// the player has to be moving to sprint
	} 
}

void ABaseCharacter::Roll(const FInputActionValue & Value)
{
	UE_LOG(LogTemp, Display, TEXT("Roll tapped!"));
}
