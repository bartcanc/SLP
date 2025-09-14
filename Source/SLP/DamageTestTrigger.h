// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBase.h"
#include "DamageTestTrigger.generated.h"

/**
 * 
 */
UCLASS()
class SLP_API ADamageTestTrigger : public ATriggerBase
{
	GENERATED_BODY()
	
public:
	ADamageTestTrigger();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* DamageTrigger;
	
};
