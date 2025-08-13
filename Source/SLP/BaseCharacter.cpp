// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent -> SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm -> SetupAttachment(StaticMeshComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera -> SetupAttachment(SpringArm);

	MoveAxisValue = 0.0f;
	StrafeAxisValue = 0.0f;	
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Velocity = GetVelocity();
	
	if (!GetCharacterMovement() -> IsFalling() and Velocity.SizeSquared() > 0.0f)
	{
		FRotator TargetRotation = FRotationMatrix::MakeFromX(Velocity).Rotator();		// target rotation from velocity vector
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 20.0f);	// interpolation
		SetActorRotation(NewRotation);
	}
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent -> BindAxis(TEXT("Move"), this, &ABaseCharacter::Move);
	PlayerInputComponent -> BindAxis(TEXT("Strafe"), this, &ABaseCharacter::Strafe);
	PlayerInputComponent -> BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent -> BindAxis(TEXT("LookRight"), this, &APawn::AddControllerYawInput);
}

void ABaseCharacter::Move(float AxisValue)
{
	MoveAxisValue = -AxisValue;

	if (Controller)
	{
		const FRotator Rotation = Controller -> GetControlRotation();		// controller rotation
		const FRotator YawRotation(0, Rotation.Yaw, 0);						// extracting yaw
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);	// extracting direction

		AddMovementInput(Direction, AxisValue);
	}
}

void ABaseCharacter::Strafe(float AxisValue)
{
	StrafeAxisValue = -AxisValue;
	
	if (Controller)
	{
		const FRotator Rotation = Controller -> GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, AxisValue);
	}
}


