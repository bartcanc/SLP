// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ladder.generated.h"

UCLASS()
class SLP_API ALadder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALadder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LadderUpCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LadderDownCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LadderUpEndCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LadderDownEndCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* LadderMesh;

	float LadderHeight;
	AActor* DetectPlayer();
	void CheckEnds();

	AActor* PlayerActor;
};
