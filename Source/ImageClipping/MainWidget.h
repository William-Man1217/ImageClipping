// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "MainWidget.generated.h"

/**
 * 
 */
UCLASS()
class IMAGECLIPPING_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* CopyButton;

	UFUNCTION()
	void OnButtonClicked();

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ComboBox;

	UFUNCTION()
	void OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* Switcher;

	UFUNCTION()
	void SwitchToWidget(int32 Index);

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* UTextureTextBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* FColorR;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* FColorG;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* FColorB;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* FColorA;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InSizeX_Text;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InSizeY_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UTexture2DResult;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FColorResult;

	UPROPERTY()
	FString CurrentSelectItems;

	UPROPERTY()
	bool isUsingUTexture2D = false;

	UFUNCTION()
	void ClipUTexture();

	UFUNCTION()
	void ClipFColor();
};
