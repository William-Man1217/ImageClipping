// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode_Main.h"
#include "PlayerController_Main.h"

AGameMode_Main::AGameMode_Main()
{
	PlayerControllerClass = APlayerController_Main::StaticClass();
}

void AGameMode_Main::BeginPlay()
{
	Super::BeginPlay();
}