// Fill out your copyright notice in the Description page of Project Settings.


#include "Racket.h"
#include <Runtime/Networking/Public/Interfaces/IPv4/IPv4Address.h>
#include <Runtime/Networking/Public/Interfaces/IPv4/IPv4Endpoint.h>
#include <Runtime/Networking/Public/Common/TcpSocketBuilder.h>
#include <Runtime/Networking/Public/Common/UdpSocketBuilder.h>
#include <IPythonScriptPlugin.h>
#include <PythonScriptTypes.h>

// Sets default values
ARacket::ARacket()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARacket::BeginPlay()
{
	FString cmdResult;
	TArray<FPythonLogOutputEntry> logOutput;
	EPythonCommandExecutionMode cmdExecMode = EPythonCommandExecutionMode::ExecuteFile;
	EPythonFileExecutionScope fileExecScope = EPythonFileExecutionScope::Private;
	FPythonCommandEx cmdEx;
	cmdEx.Command = "main.py";
	cmdEx.CommandResult = cmdResult;
	cmdEx.ExecutionMode = cmdExecMode;
	cmdEx.FileExecutionScope = fileExecScope;
	cmdEx.LogOutput = logOutput;
	
	IPythonScriptPlugin* pythonScriptPlugin = IPythonScriptPlugin::Get();

	bool status = FPlatformProcess::CreatePipe(this->readPipe, this->writePipe);
	//this->proc = FPlatformProcess::CreateProc(TEXT("python"), TEXT("C:\\Users\\RubenSD\\Documents\\UnrealEngine\\Python\\main.py"), true, false, false, nullptr, 0, nullptr, writePipe, readPipe);
	

	Super::BeginPlay();
	FIPv4Endpoint Endpoint(FIPv4Address(192, 168, 1, 40), 5557);

	ListenSocket = FUdpSocketBuilder(TEXT("SomeDescription"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(8192);

	this->initialLocation = this->GetActorLocation();
}

void ARacket::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ListenSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	FPlatformProcess::TerminateProc(this->proc, true);
}

// Called every frame
void ARacket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//FString output = FPlatformProcess::ReadPipe(this->readPipe);
	//if (output != "") {
	//	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Main.py output log: " + output);
	//	FJsonSerializableArray lines, coordsList, coords;
	//	output.ParseIntoArrayLines(lines);
	//	FString(*lines[lines.Num() - 1]).ParseIntoArray(coordsList, TEXT("["));
	//	FString coordsListStr(*coordsList[coordsList.Num() - 1]);
	//	coordsListStr.RemoveAt(coordsListStr.Len() - 1);
	//	coordsListStr.ParseIntoArray(coords, TEXT(", "));
	//	float x = FCString::Atof(*coords[0])/100.0f;
	//	float y = FCString::Atof(*coords[1])/100.0f;
	//	float z = FCString::Atof(*coords[2])/100.0f;
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%.3f %.3f %.3f"), x, y, z));
	//	this->SetActorLocation(FVector(this->initialLocation.X + x - 40, this->initialLocation.Y + y, this->initialLocation.Z + z));
	//}
	////GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Main.py output log: " + line);

	TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	TArray<uint8> ReceivedData;
	uint32 Size;
	while (ListenSocket->HasPendingData(Size))
	{
		int32 BytesRead = 0;

		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));

		ListenSocket->RecvFrom(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *targetAddr);
		//Do something with the received data

		char ansiiData[1024]; //A temp buffer for the data

		memcpy(ansiiData, ReceivedData.GetData(), BytesRead); //Assumes bytesRead is always smaller than 1024 bytes

		ansiiData[BytesRead] = 0; //Add null terminator

		FString debugData = ANSI_TO_TCHAR(ansiiData);

		//ListenSocket->RecvFrom(Recv, Size, BytesRead, *targetAddr);
		

		FJsonSerializableArray OutArray;
		debugData.ParseIntoArray(OutArray, TEXT(" "));

		
		FString dataType = *OutArray[0];		
		if(dataType.StartsWith("0"))
		{
			float x = FCString::Atof(*OutArray[1]);
			float y = FCString::Atof(*OutArray[2]);
			float z = FCString::Atof(*OutArray[3]);
			float w = FCString::Atof(*OutArray[4]);
			FQuat quat = FQuat(-x, y, -z, w).GetNormalized();

			FQuat newQuat = FQuat(quat.Rotator().Add(0, -90, 0));

			this->SetActorRotation(newQuat.GetNormalized());
		}
		else if (dataType.StartsWith("1"))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Message by UDP: " + debugData);
			FVector loc = this->GetActorLocation();
			this->vx -= FCString::Atof(*OutArray[1]) * DeltaTime;
			this->vy += FCString::Atof(*OutArray[2]) * DeltaTime;
			this->vz -= FCString::Atof(*OutArray[3]) * DeltaTime;
						
			//this->SetActorLocation(FVector(loc.X + vx * DeltaTime, loc.Y + vy * DeltaTime, loc.Z + vz * DeltaTime));
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Main.py output log: " + debugData);
			float x = FCString::Atof(*OutArray[1]) / 100.0f;
			float y = FCString::Atof(*OutArray[2]) / 100.0f;
			float z = FCString::Atof(*OutArray[3]) / 100.0f;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%.3f %.3f %.3f"), x, y, z));
			this->SetActorLocation(FVector(this->initialLocation.X + x - 40, this->initialLocation.Y + y, this->initialLocation.Z + z));
		}
	} 
}

// Called to bind functionality to input
void ARacket::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}