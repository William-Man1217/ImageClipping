# ImageClipping
 
This Unreal project is to implement a function that allows user to clip the image (either in TArray<FColors> or UTexture2D) and paste it to other Windows Applications.

The main features of the project is coded within the MainWidget.cpp file.

# Implementation

There are serveral functions implemented in the project:
1. 
```
std::vector<uint8_t> ConvertTexture2DToVector(UTexture2D* Texture, int32& OutWidth, int32& OutHeight, FString& msg)
```
This function is used to pass the UTexture2D and by manipulating its platform data and mips, a std::vector<uint8_t> datatype can be formed for later clipping.
Within the process, the texture height and width are also withdrawn from the texture's platform data.

Full Code Block
```
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
```

2.
```
FString CopyToClipboardAsDIB(const std::vector<uint8_t>& pixelData, int width, int height)
```
This function is used to manipulate the Windows clipboard. It will first open and empty the clipboard
Then allocate a global memory object for the DIB data then lock the object
Followed by creating the bitmap info header in DIB format and copy it to the memory
The pixel data (in BGRA) will be copied into the global memory after.
Finally, the function will unlock the global memory and set the clipboard data as DIB

Full Code Block
```
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
```

# Testing

For Testing the feature in the input format of TArray<FColors>:
1. Download the release version
2. Start the ImageClipping.exe
3. In the Widget menu, select "InImageData"
4. Fill in the Textboxes with desirable data (e.g. R=50, G=140, B=255, A=10, InSizeX=400, InSizeY=400)
5. Click on the "Clip to clipboard" button
6. Notice that a message of "Copied to Clipboard Successfully" will show up if the image is successfully clipped.
7. Paste the clipboard image to other Windows Application (e.g. Paint, MsWord, Whatsapp)

For Testing the feature in the input format of UTexture2D:
1. Clone the Unreal project
2. Generate the Visual Studio Project Files
3. Run the created ImageClipping.sln
4. Build the Solution
5. Open the Unreal project
6. Click on "Play this level in a new window"
7. In the Widget menu, select "UTexture2D"
8. Input the desired UTexture2D (default is /Engine/EngineMaterials/HairDebugColor.HairDebugColor)
9. Click on the "Clip to clipboard" button
10. Notice that a message of "Copied to Clipboard Successfully" will show up if the image is successfully clipped.
11. Paste the clipboard image to other Windows Application (e.g. Paint, MsWord, Whatsapp)

# Challenges Faced

This project faces serveral Challenges during development:
1. The handle of the UTexture2D
As the user is allowed to input an UTexture2D to the application, the correct handling of the texture is required.
Therefore, in order to correctly convert the texture data to a clippable data type, a lot of researches are conducted.

2. The conversion of the std::vector<uint8> to bitmap
At the middle of the project development, the bitmap clipped to the clipboard is always empty or corrupted.
A lot of debugs and testings had been carried to figure out how to correctly convert the data to the bitmap without any data corruption.