// Fill out your copyright notice in the Description page of Project Settings.

#include "OpenDoor.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerVolume.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#define OUT


// Sets default values for this component's properties
UOpenDoor::UOpenDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UOpenDoor::BeginPlay()
{
	Super::BeginPlay();
	Owner = GetOwner();

	if (!BIsPressurePlate())
	{
		UE_LOG(LogTemp, Error, TEXT("%s missing a PressurePlate"), *Owner->GetName())
	}
}


bool UOpenDoor::BIsPressurePlate()
{
	return true;
}

float UOpenDoor::GetTotalMassOnThePressurePlate(ATriggerVolume* PressurePlate)
{
	float TotalMass = 0.f;
	// Find all the overlapping actors
	TArray<AActor*> OverlappingActors;
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	//get the total mass of the element in the array
	for (const auto& Actor : OverlappingActors) 
	{
		TotalMass += Actor->FindComponentByClass<UPrimitiveComponent>()->GetMass();
		UE_LOG(LogTemp,Warning,TEXT("%s is on the pressure plate"), *Actor->GetName())
	}
	return TotalMass;
}

// Called every frame
void UOpenDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!BIsPressurePlate()) { return; }

	// Poll the pressure plate if there is enough mass on the pressure plate
	bool BOpenDoor = false;
	for (auto* const PressurePlate : PressurePlateArray)
	{
		if (GetTotalMassOnThePressurePlate(PressurePlate) > TriggerMass)
		{
			BOpenDoor = true;
		}
		else
		{
			BOpenDoor = false;
			break; //TODO find another solution
		}
	}

	if (BOpenDoor)
	{
		OnOpen.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("%s actived"), *GetOwner()->GetName())
	}

	else
	{
		OnClose.Broadcast();
	}
}

