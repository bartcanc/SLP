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
#include "Animation/AnimBlueprint.h"	

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
	bIsGrounded = true;
	bIsPlayerRunning = false;
	bResetCamera = false;
	bIsRolling = false;
	bCanRoll = true;
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
	
	ApplyMovement();
	MoveAxisValue = 0.0f;
    StrafeAxisValue = 0.0f;
	bIsGrounded = !GetCharacterMovement() -> IsFalling();
	
	if(bIsLockedOn)
	{
		HandleLockOnCamera(DeltaTime);
		ChangeCameraPositionWhenLockedOn(DeltaTime);
	}
	else
	{
		HandleMovement(DeltaTime);
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

	PlayerEIComponent -> BindAction(InputRoll, ETriggerEvent::Completed, this, &ABaseCharacter::StartRoll);
	PlayerEIComponent -> BindAction(InputRunDash, ETriggerEvent::Triggered, this, &ABaseCharacter::Sprint);

}

void ABaseCharacter::HandleLockOnCamera(float DeltaTime)
{
	FVector FocusPoint = NearestActors[ClosestEnemy] -> GetActorLocation();
	FVector CameraLocation = Camera -> GetComponentLocation();

	FVector DirectionVector = FocusPoint - CameraLocation;
	if(DirectionVector.Length() > LockOnRange or OutHits[0].GetActor()->ActorHasTag("IsWall")) 	// if the player leaves the lock on range or a wall is the nearest actor
	{
		bIsLockedOn = false;		// leave the locked on state
		NearestActors.Empty();
		SpringArm -> SetRelativeLocation(FVector(0, 0, 80));
		return;
	}
	FRotator TargetCameraRotation = DirectionVector.Rotation();

	FRotator CurrentControlRotation = PlayerController -> GetControlRotation();
	FRotator NewControlRotation = FMath::RInterpTo(CurrentControlRotation, TargetCameraRotation, DeltaTime, 10.0f);

	PlayerController -> SetControlRotation(NewControlRotation);
    
	HandleMovementWhenLockedOn(DeltaTime, NewControlRotation);
}

void ABaseCharacter::DoTrace()
{
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
				if(Hit.GetActor() -> ActorHasTag("IsWall"))
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit a wall!"));
					break;
				}
				if(Hit.GetActor() -> ActorHasTag("Enemy")) NearestActors.AddUnique(Hit.GetActor());
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Nearest Actors Count: %d"), NearestActors.Num());
	}
}

float ABaseCharacter::GetSpeed() const	// for animation blueprint
{
	float Speed = abs(GetVelocity().GetSafeNormal().Size());
	return bIsPlayerRunning ? Speed : Speed * 0.7;
}

float ABaseCharacter::GetDirection() const	// for animation blueprint
{
	return StrafeAxisValue;
}

bool ABaseCharacter::IsLockedOn() const	// for animation blueprint
{
	return bIsLockedOn;
}

void ABaseCharacter::ToggleEnemyWhenLockedOn(float AxisValue)
{
	if(!NearestActors.IsEmpty())
	{
		ClosestEnemy = abs(++ClosestEnemy) % NearestActors.Num();	// ensure that the index doesn't go out of bounds
	}
}

void ABaseCharacter::ChangeCameraPositionWhenLockedOn(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("RightVector: %s"), *GetActorRightVector().ToString());

	float Direction = 1;
	if(bCameraOnTheRightLockedOn)	Direction = 1;	// right
	else Direction = -1;	// left
	
	FVector TLocation = FMath::VInterpTo(SpringArm -> GetRelativeLocation(), FVector(0, Direction*20, 80), DeltaTime, 5.f);
	SpringArm -> SetRelativeLocation(TLocation);
}

void ABaseCharacter::HandleMovementWhenLockedOn(float DeltaTime, FRotator NewControlRotation)
{	
// if the player is not running
	if(!bIsPlayerRunning)
	{
//  	set actor rotation to face the lock on point
		SetActorRotation(FRotator(0, NewControlRotation.Yaw, 0));
	}
	else
	{
//  	set actor rotation to the new rotation
		HandleMovement(DeltaTime);	// normal rotation when running
	}
}

void ABaseCharacter::HandleMovement(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	FRotator TargetRotation = FRotationMatrix::MakeFromX(Velocity).Rotator();		// target rotation from velocity vector
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 20.0f);	// interpolation

	// if not locked on
	if(bIsGrounded and Velocity.SizeSquared() > 0.0f) 
	{
		// if not falling and velocity is greater than 0
		//  	set actor rotation to the new rotation
		SetActorRotation(NewRotation);
	}	
}

void ABaseCharacter::Move(const FInputActionValue & Value)
{
    MoveAxisValue = Value.Get<float>();
}

void ABaseCharacter::Strafe(const FInputActionValue & Value)
{
    StrafeAxisValue = Value.Get<float>();
}

void ABaseCharacter::ApplyMovement()
{
    if (!PlayerController) return;

    const FRotator Rotation = PlayerController->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// determining the direction
    FVector InputVector = (ForwardDirection * MoveAxisValue) + (RightDirection * StrafeAxisValue);

    if (InputVector.SizeSquared() > 0.0f)
    {
        InputVector = InputVector.GetSafeNormal();

        float SpeedScale = bIsPlayerRunning ? 1.0f : 0.7f;
        AddMovementInput(InputVector, SpeedScale);
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
			SpringArm -> SetRelativeLocation(FVector(0, 0, 80));
		}
		else
		{				// lock on
			UE_LOG(LogTemp, Warning, TEXT("Locked on!"));
			bIsLockedOn = true;
			SpringArm -> SetRelativeLocation(FVector(0, 20, 80));
		}
		ClosestEnemy = 0;
	}
	else{
		// TODO: Reset camera to default position (when pressing MOUSE3 if not locked on)
		PlayerController -> SetControlRotation(GetActorForwardVector().Rotation());	// doesnt work when facing a wall
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

	if(!bIsLockedOn) AddControllerYawInput(AxisValue);
	else ToggleEnemyWhenLockedOn(AxisValue);
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

void ABaseCharacter::StartRoll(const FInputActionValue & Value)
{
	if(!bIsRolling and bCanRoll)
	{
		bIsRolling = true;
		bCanRoll = false;
		GetWorld() -> GetTimerManager().SetTimer(RollTimer, this, &ABaseCharacter::SetIsRolling, InvincibilityTime, false);
		UE_LOG(LogTemp, Display, TEXT("Roll started!"));
		PerformRoll();
	}
}

void ABaseCharacter::PerformRoll()
{	// TODO: when dodging up the edge of a slope, the player is launched far away
    if (!bIsGrounded or !bIsRolling) return;

    FVector RollDirection;
    float RollStrength;

	const FFindFloorResult& FloorResult = GetCharacterMovement()->CurrentFloor;
    FVector FloorNormal = FloorResult.IsWalkableFloor() ? FloorResult.HitResult.ImpactNormal : FVector::UpVector;

    if (GetVelocity().SizeSquared() == 0.0f) // backstep
    {
       	FVector Forward = FVector(Camera->GetForwardVector().X, Camera->GetForwardVector().Y, 0).GetSafeNormal();
        RollDirection = FVector::VectorPlaneProject(Forward, FloorNormal).GetSafeNormal();
        RollStrength = BackstepModifier;
    }
    else // directional roll
    {
		FVector CurrentVelocity = GetVelocity();
        FVector HorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);
		RollDirection = FVector::VectorPlaneProject(HorizontalVelocity.GetSafeNormal(), FloorNormal).GetSafeNormal();
        RollStrength = RollModifier;
	}

    LaunchCharacter(RollDirection * RollStrength, true, false);
}

void ABaseCharacter::SetIsRolling()
{
    bIsRolling = false;
	GetWorld() -> GetTimerManager().SetTimer(RollCooldownTimer, this, &ABaseCharacter::SetCanRoll, InvincibilityTime, false);
}

void ABaseCharacter::SetCanRoll()
{
	bCanRoll = true;
}