// Copyright Epic Games, Inc. All Rights Reserved.

#include "TennisGameMode.h"
#include "TennisCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATennisGameMode::ATennisGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
