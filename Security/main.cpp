// Security.cpp : Defines the entry point for the console application.
//
#include "CSmtp.h"
#include <iostream>
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <NuiApi.h>
#include <sstream>
using namespace std;

void takePicture();
NUI_LOCKED_RECT getLockedRect();

INuiSensor * sensor;
HANDLE han2(INVALID_HANDLE_VALUE);

int main()
{
	NuiCreateSensorByIndex(0, &sensor);
	HRESULT hr = sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_SKELETON);
	//|NUI_INITIALIZE_FLAG_USES_DEPTH|NUI_INITIALIZE_FLAG_USES_SKELETON
	if (FAILED(hr))
	{
		cout << "ASfdsfLD" << endl;
	}
	sensor->NuiCameraElevationSetAngle(-1);

	NUI_SKELETON_FRAME skelFrame;
	//NUI_IMAGE_FRAME imgFrame;
	HANDLE han1 = CreateEvent( NULL, TRUE, FALSE, NULL );
	//han2 = HANDLE(INVALID_HANDLE_VALUE);
	hr = sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_1280x960,0,2,han1,&han2);
	if (FAILED(hr))
	{
		cout << "Failed" << endl;
	}
	


	//sensor->NuiSkeletonGetNextFrame(0, &skelFrame);
	int counter = 0;

	int angle = -5;
	bool noPersonDetected = true;
	BYTE * frame;
	//frame1 = static_cast<BYTE *> (getLockedRect().pBits);
	int sum;
	int temp = 0;
	while (noPersonDetected)
	
	{
		sensor->NuiSkeletonGetNextFrame(0, &skelFrame);
		

		if (skelFrame.SkeletonData[0].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[1].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[2].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[3].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[4].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[5].eTrackingState == NUI_SKELETON_TRACKED)
		{
			noPersonDetected = false;
		}
		

		frame = static_cast<BYTE *> (getLockedRect().pBits);


		sum = 0;
		int i;
		for(i = 0; i < 1228800;i++)
		{
			 sum += frame[i];
		}
		//cout << sum << endl;
		if (temp != 0)
		{
			if (abs(sum-temp) >= 100000) noPersonDetected = false;
			cout << sum - temp << endl;
		}
		temp = sum;
			
		
		

		
		/*if (counter == 1000000)
		{
			sensor->NuiCameraElevationSetAngle(angle);
			counter = 0;
			angle = -angle;
		}
		counter++;
		*/
	}
	
	takePicture();


	return 0;
}

NUI_LOCKED_RECT getLockedRect()
{
		NUI_IMAGE_FRAME imgFrame;
			bool frameNull = true;
		
			HRESULT hrr; 
			while (frameNull)
			{
				hrr = sensor->NuiImageStreamGetNextFrame(han2, 0, &imgFrame);
				if (FAILED(hrr))
				{
					//cout << "ERROR! : " << hrr << endl;
					//cout << "S_FALSE: " << S_FALSE << endl;
					//cout << "E_NUI_FRAME_NO_DATA: " << E_NUI_FRAME_NO_DATA << endl;
					//cout << "E_POINTER: " << E_POINTER << endl;
					
				}
				else
				{
					cout << "SUCCESS!" << endl;
					frameNull = false;
				}
			}
		INuiFrameTexture * txture = imgFrame.pFrameTexture;
		NUI_LOCKED_RECT lockedRect;
		txture->LockRect(0,&lockedRect,NULL,0);
		sensor->NuiImageStreamReleaseFrame(han2, &imgFrame );
		
		//txture->UnlockRect(0);
		return lockedRect;

}

void takePicture()
{
		
		
		NUI_LOCKED_RECT lockedRect = getLockedRect();
		
		//char* filename = filename = "test.bmp";
	    //FILE *pFile = fopen(filename, "wb");
		BITMAPINFOHEADER BMIH = {0};
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
        //Is this number okay?
		BMIH.biBitCount = 32;
        BMIH.biPlanes = 1;
        BMIH.biCompression = BI_RGB;
        BMIH.biWidth = 1280;
        BMIH.biHeight = -960;
        //BMIH.biSizeImage = ((((BMIH.biWidth * BMIH.biBitCount)  + 31) & ~31) >> 3) * BMIH.biHeight;
        BMIH.biSizeImage = BMIH.biWidth*(-BMIH.biHeight*BMIH.biBitCount/8);

		BITMAPFILEHEADER bmfh = {0};
        int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
        LONG lImageSize = BMIH.biSizeImage;
        LONG lFileSize = nBitsOffset + lImageSize;
        //bmfh.bfType = 'B'+('M'<<8);
        bmfh.bfType = 0x4D42;
		bmfh.bfOffBits = nBitsOffset;
        bmfh.bfSize = lFileSize;
        //bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
		int Number = GetTickCount();       // number to be converted to a string

		string Result;          // string which will contain the result
	
		ostringstream convert;   // stream used for the conversion

		convert << Number;      // insert the textual representation of 'Number' in the characters in the stream

		Result = convert.str();
		string s = string("Pics/Pic")+Result+string(".png");
		std::wstring stemp = std::wstring(s.begin(), s.end());
		LPCWSTR sw = stemp.c_str();
		LPCWSTR a = stemp.c_str();

        HANDLE hFile = CreateFileW(a, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Return if error opening file
    if (NULL == hFile) 
    {
        cout << E_ACCESSDENIED << endl;
    }

    DWORD dwBytesWritten = 0;
    
    // Write the bitmap file header
    if ( !WriteFile(hFile, &bmfh, sizeof(bmfh), &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        cout << E_FAIL << endl;
    }
    
    // Write the bitmap info header
    if ( !WriteFile(hFile, &BMIH, sizeof(BMIH), &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        cout << E_FAIL << endl;
    }
    
    // Write the RGB Data
    if ( !WriteFile(hFile, static_cast<BYTE *>(lockedRect.pBits), BMIH.biSizeImage, &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        cout << E_FAIL << endl;
    }    

		bool bError = false;

	try
	{
		CSmtp mail;

#define test_gmail_tls

#if defined(test_gmail_tls)
		mail.SetSMTPServer("smtp.gmail.com",587);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(USE_SSL);
#endif

		mail.SetLogin("testthiscodeplz@gmail.com");
		mail.SetPassword("marcuswater");
  		mail.SetSenderName("Test");
  		mail.SetSenderMail("testthiscodeplz@gmail.com");
  		mail.SetReplyTo("");
  		mail.SetSubject("The message");
  		mail.AddRecipient("Kevintasta@gmail.com");
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		mail.SetXMailer("The Bat! (v3.02) Professional");
  		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("Someone has been trying to steal your things.");
		mail.AddMsgLine("A picture is attached for your conveniance.");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("WHALE Security system.");

		
		const char * c = s.c_str();

  		mail.AddAttachment(c);
  		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
	}
	catch(ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		bError = true;
	}
	if(!bError)
		std::cout << "Mail was send successfully.\n";
}

