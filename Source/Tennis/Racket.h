// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Racket.generated.h"

UCLASS()
class TENNIS_API ARacket : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARacket();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	FSocket* ListenSocket;
	bool bConnectionEstablished = false;
	bool bBytesRead = false;
	
	float vx = 0.0f;
	float vy = 0.0f;
	float vz = 0.0f;

	void* readPipe = nullptr;
	void* writePipe = nullptr;

	FProcHandle proc;

	FVector initialLocation;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
