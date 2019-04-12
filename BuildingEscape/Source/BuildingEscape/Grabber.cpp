 // Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Public/CollisionQueryParams.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Engine/Classes/Components/PrimitiveComponent.h"

#define OUT


// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsComponent();
	SetupInputComponent();
}

//Find attached Physics Handle
void UGrabber::FindPhysicsComponent()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

	if (!PhysicsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("%s hasn't PhysicsHandleComponent"), *(GetOwner()->GetName()))
	}
}

//Setup the attached Input Component
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();

	if (InputComponent)
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s hasn't InputComponent"), *(GetOwner()->GetName()))
	}
}

//Return hit for first physics body in reach
FHitResult UGrabber::GetFirstPhysicsBodyInReach() const
{
	FPlayerViewPointInfo PlayerViewPointInfo = GetPlayerViewPointInfo();

	/// line-trace out the reach distance
	FHitResult Hit;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		PlayerViewPointInfo.LineTraceStart,
		PlayerViewPointInfo.LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);

	return Hit;
}

// get player view point location, rotation and line-trace end
FPlayerViewPointInfo UGrabber::GetPlayerViewPointInfo() const
{
	FPlayerViewPointInfo PlayerViewPointInfo;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointInfo.LineTraceStart,
		OUT PlayerViewPointRotation
	);

	///Calculate the line-trace end
	PlayerViewPointInfo.LineTraceEnd = PlayerViewPointInfo.LineTraceStart + PlayerViewPointRotation.Vector() * Reach;
	
	return PlayerViewPointInfo;
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FPlayerViewPointInfo PlayerViewPointInfo = GetPlayerViewPointInfo();

	if (!PhysicsHandle) { return; }
	if (PhysicsHandle->GetGrabbedComponent())
	{
		//move the object we're holding
		PhysicsHandle->SetTargetLocation(PlayerViewPointInfo.LineTraceEnd);
	}
}

void UGrabber::Grab()
{
	///Try and reach any actors with Physics Body Collision channel set 
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();

	///If we hit something then attach a Physics Handle
	if (HitResult.GetActor())
	{
		if (!PhysicsHandle) { return; }
		PhysicsHandle->GrabComponent(
			ComponentToGrab, 
			NAME_None, 
			ComponentToGrab->GetOwner()->GetActorLocation(), 
			true //allow rotation
		);
	}

	return;
}

void UGrabber::Release()
{
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
	return;
}
