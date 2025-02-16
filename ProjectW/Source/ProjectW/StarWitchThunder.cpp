// Fill out your copyright notice in the Description page of Project Settings.


#include "StarWitchThunder.h"

// Sets default values
AStarWitchThunder::AStarWitchThunder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStarWitchThunder::BeginPlay()
{
	Super::BeginPlay();
	FTimerHandle timer;
	GetWorldTimerManager().SetTimer(timer, this, &AStarWitchThunder::DestroyMyself, 1.0f, false);
}

// Called every frame
void AStarWitchThunder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AStarWitchThunder::DestroyMyself()
{
	this->Destroy();
}