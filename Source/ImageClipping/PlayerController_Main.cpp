// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController_Main.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "MainWidget.h"
#include <Components/WidgetComponent.h>

APlayerController_Main::APlayerController_Main()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainWidget(TEXT("/Game/Asset/MyMainWidget"));
	if (MainWidget.Class != nullptr)
	{
		MainWidgetClass = MainWidget.Class;
	}

}

void APlayerController_Main::BeginPlay()
{
	Super::BeginPlay();

	if (MainWidgetClass)
	{
		MainUserWidget = CreateWidget<UMainWidget>(this, MainWidgetClass);
		if (MainUserWidget)
		{
			MainUserWidget->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;
		}
	}
}
	