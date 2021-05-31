// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RealAttemptProjectCharacter.generated.h"

UCLASS(config=Game)
class ARealAttemptProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ARealAttemptProjectCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	//Tells if the player is swinging from a web
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	bool IsSwinging;

	//Value of the swinging Point
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	FVector OptimalSwingPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Formula)
	float VelocityClampMin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Formula)
	float VelocityClampMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Formula)
	float ReduceForceByFactorOfX;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:
	//Applies de forces needed to generate the arc motion when swinging
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	FVector SwingArcForceFormula();

	//Calculates the optimal point to swing from whenever the player presses the Swing action.
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	FVector CalculateOptimalSwingPoint();

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	FRotator CalculateSwingSideAngle();
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

