// Copyright Epic Games, Inc. All Rights Reserved.

#include "RealAttemptProjectGameMode.h"
#include "RealAttemptProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARealAttemptProjectGameMode::ARealAttemptProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
