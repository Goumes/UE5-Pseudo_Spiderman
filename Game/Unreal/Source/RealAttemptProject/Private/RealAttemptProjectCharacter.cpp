// Copyright Epic Games, Inc. All Rights Reserved.

#include "RealAttemptProjectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

//////////////////////////////////////////////////////////////////////////
// ARealAttemptProjectCharacter

ARealAttemptProjectCharacter::ARealAttemptProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	VelocityClampMin = 400.f;
	VelocityClampMax = 2000.f;
	ReduceForceByFactorOfX = 4.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARealAttemptProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARealAttemptProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARealAttemptProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARealAttemptProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARealAttemptProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARealAttemptProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARealAttemptProjectCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARealAttemptProjectCharacter::OnResetVR);
}


void ARealAttemptProjectCharacter::OnResetVR()
{
	// If RealAttemptProject is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in RealAttemptProject.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARealAttemptProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ARealAttemptProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ARealAttemptProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARealAttemptProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARealAttemptProjectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARealAttemptProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

/* Placeholder function that calculates an attatchment point based on the player's current location, the camera's forward vector and a set height. It does not take into consideration the environement
* It finds the player's current velocity, increase it by 20% and clamps it to avoid extremes. Then it is multiplied by the camera's forward vector to throw a web in that direction and finally adds everything up including a set Z value to shoot the webs high.
*/
FVector ARealAttemptProjectCharacter::CalculateOptimalSwingPoint()
{
	FVector ZVelocity = FVector(0.f, 0.f, (FVector(0.f, 0.f, GetVelocity().Z).Size() * 1.2)).GetClampedToSize(1000.f, 3000.f);
	FVector SetDistanceAboveCharacter = FVector(0.f, 0.f, 1200.f);
	
	return GetActorLocation() + (GetFollowCamera()->GetForwardVector().GetSafeNormal()* 1500.f) + SetDistanceAboveCharacter;
}

/*Calculates the force to give to the character based on the following formula:
* Retrieve the dot product of the clamped velocity vector and the result vector of both the player's and the web attatchment location.
* Then multiply the dot product with the normalized vector of the last step (Player's location and web attatchment location).
* Lastly multiply that result for -2 and divide everything by the factor of reduction provided as a class property.
* SwingArcForce = ((vel . VectorPlayerPosSwingLoc) * Normalised VectorPlayerPosSwingLoc) * -2)/ReduceForceByFactorOfX)
*/
FVector ARealAttemptProjectCharacter::SwingArcForceFormula()
{
	FVector PlayerToBuilding = FVector(OptimalSwingPoint - GetActorLocation());
	float DotProductVelocityL = FVector::DotProduct(PlayerToBuilding, GetVelocity().GetClampedToSize(VelocityClampMin, VelocityClampMax));
	return FVector(((DotProductVelocityL* PlayerToBuilding.GetSafeNormal()) * -2) / ReduceForceByFactorOfX);
}


/* Calculates the rotation angle from the swing of the web.
* It takes the normalized PlayerToBuilding vector and the Cross product of it with the normalized velocity to create a rotator from ZY.
*/
FRotator ARealAttemptProjectCharacter::CalculateSwingSideAngle()
{
	FVector PlayerToBuilding = FVector(OptimalSwingPoint - GetActorLocation().GetSafeNormal());
	return UKismetMathLibrary::MakeRotFromZY(PlayerToBuilding, FVector::CrossProduct(GetVelocity().GetSafeNormal(), PlayerToBuilding) * -1); 
}