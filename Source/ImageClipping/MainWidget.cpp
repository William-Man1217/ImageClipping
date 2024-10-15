// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidget.h"
#include "Windows.h"
#include "WinUser.h"
#include "vector"
#include <ImageUtils.h>
#include "Components/Button.h"
#include <cstdint>
#include "Engine/Texture2D.h"
#include "RenderUtils.h"
#include "Misc/DefaultValueHelper.h"

void UMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CopyButton = Cast<UButton>(this->GetWidgetFromName(TEXT("CopyButton")));
    ComboBox = Cast<UComboBoxString>(this->GetWidgetFromName(TEXT("ComboBox")));
    Switcher = Cast<UWidgetSwitcher>(this->GetWidgetFromName(TEXT("Switcher")));
    UTextureTextBox = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("UTextureTextBox")));
    FColorR = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("FColorR")));
    FColorG = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("FColorG")));
    FColorB = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("FColorB")));
    FColorA = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("FColorA")));
    InSizeX_Text = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("InSizeX_Text")));
    InSizeY_Text = Cast<UEditableTextBox>(this->GetWidgetFromName(TEXT("InSizeY_Text")));
    FColorResult = Cast<UTextBlock>(this->GetWidgetFromName(TEXT("FColorResult")));
    UTexture2DResult = Cast<UTextBlock>(this->GetWidgetFromName(TEXT("UTexture2DResult")));

    if (CopyButton)
        CopyButton->OnClicked.AddDynamic(this, &UMainWidget::OnButtonClicked);

    if (ComboBox)
        ComboBox->OnSelectionChanged.AddDynamic(this, &UMainWidget::OnSelectionChanged);

    if (Switcher)
        Switcher->SetActiveWidgetIndex(0);
}

void UMainWidget::OnButtonClicked()
{
    if (isUsingUTexture2D)
        ClipUTexture();
    else
        ClipFColor();
    // Handle button click event
}

void UMainWidget::OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    //Switch Widget Panel Based on ComboBox: 0 for UTexture2D input, 1 for RGBA data
    if (SelectedItem == "UTexture2D")
    {
        CurrentSelectItems = "UTexture2D";
        isUsingUTexture2D = true; 
        SwitchToWidget(0);
    }
    else
    {
        CurrentSelectItems = "InImageData";
        isUsingUTexture2D = false;
        SwitchToWidget(1);
    }
}

void UMainWidget::SwitchToWidget(int32 Index)
{
    if (Switcher)
        Switcher->SetActiveWidgetIndex(Index);
}

// Function to convert a UTexture2D to a std::vector<uint8_t> (BGRA format) and return width & height
std::vector<uint8_t> ConvertTexture2DToVector(UTexture2D* Texture, int32& OutWidth, int32& OutHeight, FString& msg)
{
    // Ensure the texture is valid
    if (!Texture)
    {
        msg = "Invalid texture pointer.";
        //UE_LOG(LogTemp, Error, TEXT("Invalid texture pointer."));
        OutWidth = 0;
        OutHeight = 0;
        return {};
    }

    // Ensure the texture has valid platform data and mip maps
    if (!Texture->PlatformData || !Texture->PlatformData->Mips.Num())
    {
        msg = "Texture has no valid platform data or mips.";
        //UE_LOG(LogTemp, Error, TEXT("Texture has no valid platform data or mips."));
        OutWidth = 0;
        OutHeight = 0;
        return {};
    }

    // Get the width and height from the texture's platform data
    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
    OutWidth = Mip.SizeX;
    OutHeight = Mip.SizeY;

    // Lock the texture data for reading
    void* Data = Mip.BulkData.Lock(LOCK_READ_ONLY);
    if (!Data)
    {
        msg = "Failed to lock the texture data.";
        //UE_LOG(LogTemp, Error, TEXT("Failed to lock the texture data."));
        OutWidth = 0;
        OutHeight = 0;
        return {};
    }

    uint8_t* PixelData = static_cast<uint8_t*>(Data);
    std::vector<uint8_t> OutVector;
    OutVector.reserve(OutWidth * OutHeight * 4);

    // Copy the BGRA data into the std::vector
    for (int32 i = 0; i < OutWidth * OutHeight * 4; ++i)
    {
        OutVector.push_back(PixelData[i]);
    }

    Mip.BulkData.Unlock();

    return OutVector;
}

FString CopyToClipboardAsDIB(const std::vector<uint8_t>& pixelData, int width, int height)
{

    if (!OpenClipboard(nullptr))
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to open the clipboard."));
        return "Failed to open the clipboard.";
    }

    if (!EmptyClipboard())
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to empty the clipboard."));
        CloseClipboard();
        return "Failed to empty the clipboard.";
    }

    int rowStride = width * 4;  // 4 bytes per pixel for BGRA
    int imageSize = rowStride * height;

    // Allocate a global memory object for the DIB data
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + imageSize);
    if (!hGlobal)
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to allocate global memory."));
        CloseClipboard();
        return "Failed to allocate global memory.";
    }

    // Lock the global memory object
    void* pGlobalData = GlobalLock(hGlobal);
    if (!pGlobalData)
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to lock global memory."));
        GlobalFree(hGlobal);
        CloseClipboard();
        return "Failed to lock global memory.";
    }

    // Set up the BMP info header in DiB format
    BITMAPINFOHEADER bmpInfoHeader = {};
    bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfoHeader.biWidth = width;
    bmpInfoHeader.biHeight = height;
    bmpInfoHeader.biPlanes = 1;
    bmpInfoHeader.biBitCount = 32;  // 32 bits per pixel (BGRA), 24 bits for BGR
    bmpInfoHeader.biCompression = BI_RGB;
    bmpInfoHeader.biSizeImage = imageSize;

    // Copy the BMP info header into the global memory
    memcpy(pGlobalData, &bmpInfoHeader, sizeof(BITMAPINFOHEADER));

    // Copy the pixel data (BGRA) into the global memory, row by row (bottom-up)
    uint8_t* pPixelData = static_cast<uint8_t*>(pGlobalData) + sizeof(BITMAPINFOHEADER);
    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            int srcIndex = (y * width + x) * 4;
            *pPixelData++ = pixelData[srcIndex];     // B
            *pPixelData++ = pixelData[srcIndex + 1]; // G
            *pPixelData++ = pixelData[srcIndex + 2]; // R
            *pPixelData++ = pixelData[srcIndex + 3]; // A
        }
    }

    // Unlock the global memory object
    GlobalUnlock(hGlobal);

    // Set the clipboard data as DIB
    if (!SetClipboardData(CF_DIB, hGlobal))
    {
        //UE_LOG(LogTemp, Error, TEXT("Failed to set clipboard data."));
        GlobalFree(hGlobal);
        CloseClipboard();
        return "Failed to set clipboard data.";
    }

    CloseClipboard();
    return "Copied to clipboard successfully.";
    //UE_LOG(LogTemp, Warning, TEXT("DIB data (with alpha) copied to clipboard successfully."));
}

void UMainWidget::ClipUTexture()
{
    UTexture2D* TestTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL,
        *(UTextureTextBox->GetText().ToString())));

    // Check if the texture was found
    if (!IsValid(TestTexture))
    {
        UTexture2DResult->SetText(FText::FromString("Incorrect Input or Texture Unavailable"));
        //UE_LOG(LogTemp, Warning, TEXT("Texture Unavailable"));
        return;
    }

    int32 TextureWidth = 0;
    int32 TextureHeight = 0;

    FString msg;

    // Convert the texture to a std::vector<uint8_t> in BGRA format=
    std::vector<uint8_t> TexturePixelData = ConvertTexture2DToVector(TestTexture, TextureWidth, TextureHeight, msg);

    if (TexturePixelData.empty())
    {
        UTexture2DResult->SetText(FText::FromString(msg));
        return;
    }

    if (!TexturePixelData.empty() && TextureWidth > 0 && TextureHeight > 0)
    {
        //Call the copy to clipbard function and use the generated std::vector<uint8> as parameter
        msg = CopyToClipboardAsDIB(TexturePixelData, TextureWidth, TextureHeight);
        UTexture2DResult->SetText(FText::FromString(msg));
    }
}

bool RGBATextToInteger(const TArray<UEditableTextBox*> Texts, TArray<int8>& ColorsValue)
{
    for (int32 i = 0; i < Texts.Num(); ++i)
    {
        FString InputString = Texts[i]->GetText().ToString();
        int32 ParsedInt32;
        int8 ParsedInt8;

        // Check if the input is a valid int32
        bool bIsValidInt32 = FDefaultValueHelper::ParseInt(InputString, ParsedInt32);

        if (ParsedInt32 >= 0 && ParsedInt32 <= 255)
        {
            ParsedInt8 = static_cast<int8>(ParsedInt32);
            ColorsValue.Add(ParsedInt8);  // Add to color values array
        }
        else
            return false;
    }
    return true;
}

bool InSizeToInteger(const TArray<UEditableTextBox*> Texts, TArray<int32>& SizeValue)
{
    for (int32 i = 0; i < Texts.Num(); ++i)
    {
        FString InputString = Texts[i]->GetText().ToString();
        int32 ParsedInt32;

        // Check if the input is a valid int32
        bool bIsValidInt32 = FDefaultValueHelper::ParseInt(InputString, ParsedInt32);

        if (bIsValidInt32)
        {
            SizeValue.Add(ParsedInt32);  // Add to color values array
        }
        else
            return false;
    }
    return true;
}

void UMainWidget::ClipFColor()
{   
    TArray<UEditableTextBox*> ColorsText = { FColorR, FColorG, FColorB, FColorA };
    TArray<int8> ColorsValue;
    TArray<int32> SizeValue;

    //Validate and Store the input from RGBA text
    if (!RGBATextToInteger(ColorsText, ColorsValue))
    {
        FColorResult->SetText(FText::FromString("There is an input error in RGBA"));
        return;
    }

    //Validate and Store the input from input size text
    if (!InSizeToInteger({InSizeX_Text, InSizeY_Text}, SizeValue))
    {
        FColorResult->SetText(FText::FromString("There is an input error in input size"));
        return;
    }

    FColor colorcode = FColor(ColorsValue[2], ColorsValue[1], ColorsValue[0], ColorsValue[3]);

    const int32 InSizeX = SizeValue[0];
    const int32 InSizeY = SizeValue[1];

    TArray<FColor> InImageData;
    InImageData.SetNum(InSizeX * InSizeY);

    // Fill the array the desired color
    for (int32 i = 0; i < InImageData.Num(); ++i)
    {
        InImageData[i] = colorcode;
    }

    //Form a std::vector<uint_t> from the FColor array
    std::vector<uint8_t> DataArray;
    DataArray.reserve(InSizeX * InSizeY * 4);
    for (int32 i = 0; i < InImageData.Num(); ++i)
    {
        FColor color = InImageData[i];
        DataArray.push_back(color.B);
        DataArray.push_back(color.G);
        DataArray.push_back(color.R);
        DataArray.push_back(color.A);
    }

    //Call the copy to clipbard function and use the generated std::vector<uint8> as parameter
    FString msg = CopyToClipboardAsDIB(DataArray, InSizeX, InSizeY);
    FColorResult->SetText(FText::FromString(msg));

}
