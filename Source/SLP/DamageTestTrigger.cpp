// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageTestTrigger.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "BaseCharacter.h"

ADamageTestTrigger::ADamageTestTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    DamageTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageTrigger"));
    DamageTrigger -> SetupAttachment(RootComponent);
}

void ADamageTestTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void ADamageTestTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TArray<AActor*> OverlappingActors;
    DamageTrigger -> GetOverlappingActors(OverlappingActors, ABaseCharacter::StaticClass());

    if(OverlappingActors.Num() > 0)
    {
        ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(OverlappingActors[0]);
        PlayerCharacter -> ReceiveDamage(10.f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("no actors"));
    }
}



