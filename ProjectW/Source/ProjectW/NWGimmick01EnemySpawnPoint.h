// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NWGimmick01EnemySpawnPoint.generated.h"

UCLASS()
class PROJECTW_API ANWGimmick01EnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANWGimmick01EnemySpawnPoint();

public:
	FVector GetSpawnPoint(int const index) const;
	int PointNum() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MakeEditWidget = "true", AllowPrivateAccess = "true"))
	TArray<FVector> SpawnPoints;
};
