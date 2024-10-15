// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_Main.generated.h"

/**
 * 
 */
UCLASS()
class IMAGECLIPPING_API APlayerController_Main : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerController_Main();

protected:
	UPROPERTY(VisibleInstanceOnly, Category="UserMainWidget")
	class UMainWidget* MainUserWidget;

	TSubclassOf<class UUserWidget> MainWidgetClass;

	virtual void BeginPlay() override;
	
};
