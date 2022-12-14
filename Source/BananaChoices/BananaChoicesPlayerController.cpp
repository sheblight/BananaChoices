// Copyright Epic Games, Inc. All Rights Reserved.

#include "BananaChoicesPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "BananaChoicesCharacter.h"
#include "Engine/World.h"

ABananaChoicesPlayerController::ABananaChoicesPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ABananaChoicesPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if(bInputPressed)
	{
		FollowTime += DeltaTime;

		// Look for the touch location
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		if(bIsTouch)
		{
			GetHitResultUnderFinger(ETouchIndex::Touch1, ECC_Visibility, true, Hit);
		}
		else
		{
			GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		}
		HitLocation = Hit.Location;

		// Direct the Pawn towards that location
		APawn* const MyPawn = GetPawn();
		if(MyPawn)
		{
			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
			MyPawn->AddMovementInput(WorldDirection, 1.f, false);
		}
	}
	else
	{
		FollowTime = 0.f;
	}
}

void ABananaChoicesPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// bind mouse events
	//InputComponent->BindAction("Attack", IE_Pressed, this, &ABananaChoicesPlayerController::OnSetDestinationPressed);
	//InputComponent->BindAction("Attack", IE_Released, this, &ABananaChoicesPlayerController::OnSetDestinationReleased);
	
	// bind keyboard and controller events
	InputComponent->BindAxis("MoveHorizontal", this, &ABananaChoicesPlayerController::OnMoveHorizontal);
	InputComponent->BindAxis("MoveVertical", this, &ABananaChoicesPlayerController::OnMoveVertical);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ABananaChoicesPlayerController::OnTouchPressed);
	InputComponent->BindTouch(EInputEvent::IE_Released, this, &ABananaChoicesPlayerController::OnTouchReleased);

}

void ABananaChoicesPlayerController::OnSetDestinationPressed()
{
	// We flag that the input is being pressed
	bInputPressed = true;
	// Just in case the character was moving because of a previous short press we stop it
	StopMovement();
}

void ABananaChoicesPlayerController::OnSetDestinationReleased()
{
	// Player is no longer pressing the input
	bInputPressed = false;

	// If it was a short press
	if(FollowTime <= ShortPressThreshold)
	{
		// We look for the location in the world where the player has pressed the input
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		HitLocation = Hit.Location;

		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, HitLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
}

void ABananaChoicesPlayerController::OnMoveHorizontal(const float value) {
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		MyPawn->AddMovementInput(FVector(-sin(3*PI / 4.f), -cos(3*PI / 4.f), 0), value, false);
	}
}

void ABananaChoicesPlayerController::OnMoveVertical(const float value) {
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		MyPawn->AddMovementInput(FVector(sin(PI / 4.f), cos(PI / 4.f), 0), value, false);
	}
}

void ABananaChoicesPlayerController::OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = true;
	OnSetDestinationPressed();
}

void ABananaChoicesPlayerController::OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = false;
	OnSetDestinationReleased();
}
