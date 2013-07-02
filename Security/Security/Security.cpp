// Security.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <NuiApi.h>

using namespace std;

void drawPicture();

HANDLE han2;

int main()
{
	NuiCameraElevationSetAngle(0);
	NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH|NUI_INITIALIZE_FLAG_USES_SKELETON);
	NUI_SKELETON_FRAME skelFrame;
	//NUI_IMAGE_FRAME imgFrame;
	HANDLE han1 = CreateEvent( NULL, TRUE, FALSE, NULL );
	NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,0,2,han1,&han2);

	drawPicture();

	while (true)
	{
		
	}

	

	return 0;
}

void drawPicture()
{
		NUI_IMAGE_FRAME * imgFrame;
		NuiImageStreamGetNextFrame(han2, 0, &imgFrame);
		INuiFrameTexture * txture = imgFrame->pFrameTexture;
		NUI_LOCKED_RECT lockedRect;
		txture->LockRect(0,&lockedRect,NULL,0);
		
		char* filename = filename = "test.bmp";
	    FILE *pFile = fopen(filename, "wb");
		BITMAPINFOHEADER BMIH = {0};
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
        //Is this number okay?
		BMIH.biBitCount = 32;
        BMIH.biPlanes = 1;
        BMIH.biCompression = BI_RGB;
        BMIH.biWidth = 640;
        BMIH.biHeight = 480;
        BMIH.biSizeImage = ((((BMIH.biWidth * BMIH.biBitCount)  + 31) & ~31) >> 3) * BMIH.biHeight;
        
		BITMAPFILEHEADER bmfh = {0};
        int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
        LONG lImageSize = BMIH.biSizeImage;
        LONG lFileSize = nBitsOffset + lImageSize;
        bmfh.bfType = 'B'+('M'<<8);
        bmfh.bfOffBits = nBitsOffset;
        bmfh.bfSize = lFileSize;
        bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
        //Write the bitmap file header
        UINT nWrittenFileHeaderSize = fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);
        //And then the bitmap info header
        UINT nWrittenInfoHeaderSize = fwrite(&BMIH, 1, sizeof(BITMAPINFOHEADER), pFile);
        //Finally, write the image data itself 
        //-- the data represents our drawing
		if(lockedRect.Pitch != 0)
		{
			BYTE * buffer =  lockedRect.pBits;

		}	
		UINT nWrittenDIBDataSize = fwrite(imgFrame->pFrameTexture, 1,BMIH.biSizeImage, pFile);
		NuiImageStreamReleaseFrame( han2, imgFrame );
		fclose(pFile);
}

