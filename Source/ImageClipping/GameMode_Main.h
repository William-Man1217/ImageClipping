// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameMode_Main.generated.h"

/**
 * 
 */
UCLASS()
class IMAGECLIPPING_API AGameMode_Main : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGameMode_Main();

protected:
	virtual void BeginPlay() override;

};