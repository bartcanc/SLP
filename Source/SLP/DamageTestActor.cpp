// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageTestActor.h"

#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "BaseCharacter.h"

// Sets default values
ADamageTestActor::ADamageTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DamageTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageTrigger"));
    DamageTrigger -> SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADamageTestActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADamageTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    TArray<AActor*> OverlappingActors;
    DamageTrigger -> GetOverlappingActors(OverlappingActors, ABaseCharacter::StaticClass());

    if(OverlappingActors.Num() > 0)
    {
        if(!GetWorldTimerManager().IsTimerActive(DamageTimer))
		{
			GetWorld() -> GetTimerManager().SetTimer(DamageTimer, this, &ADamageTestActor::NullFun, 2.f, false);
			ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(OverlappingActors[0]);
	        PlayerCharacter -> ReceiveDamage(10.f);
		}    
    }
}

void ADamageTestActor::NullFun()
{

}
