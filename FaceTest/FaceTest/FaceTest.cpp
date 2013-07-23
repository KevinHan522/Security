// Security.cpp : Defines the entry point for the console application.
//

#include <iostream>
//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <NuiApi.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include "FaceTrackLib.h"
using namespace std;


vector<vector<int>> getClusters(bool * arr);
void populateClust(vector<int> cls, bool * ar, int seed);
//method that takes a picture and e-mails it
void takePicture(FT_WEIGHTED_RECT);
void takePicture(byte * pBits, string str);
//method to get a lockedRectangle that holds all the values needed for color capture
NUI_LOCKED_RECT getLockedRect();
void processColor();
void processDepth();


INuiSensor * sensor;
HANDLE colorHandle(INVALID_HANDLE_VALUE);
HANDLE depthHandle(INVALID_HANDLE_VALUE);
IFTImage* pColorFrame;
IFTImage* pDepthFrame;
FT_WEIGHTED_RECT *pFaces = new FT_WEIGHTED_RECT[1];
double * arrfirst = new double[640*480*4];
double * arrlast = new double[640*480*4];
int * arrfst = new int[640*480*4];
int * arrlst = new int[640*480*4];
bool * arrdiff = new bool[640*480*4];

int main()
{

	NuiCreateSensorByIndex(0, &sensor);
	//Initializes Kinect sensor. This program uses the flags to use the color and skeleton sensor
	HRESULT hrt = sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_SKELETON|NUI_INITIALIZE_FLAG_USES_DEPTH);
	if (FAILED(hrt))
	{
		cout << "Failed to initialize" << endl;
	}
	//Sets the angle of the kinect (for consistency purposes)
	sensor->NuiCameraElevationSetAngle(4);

	NUI_SKELETON_FRAME skelFrame;

	HANDLE color = CreateEvent( NULL, TRUE, FALSE, NULL );
	HANDLE depth = CreateEvent( NULL, TRUE, FALSE, NULL );

	//Opens image stream for color sensing
	hrt = sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,0,2,color,&colorHandle);
	if (FAILED(hrt))
	{
		cout << "Failed to open image stream" << endl;
	} 
	hrt = sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_320x240,0,2,depth,&depthHandle);
	if (FAILED(hrt))
	{
		cout << "Failed to open depth stream" << endl;
	}

	/* Values here used for moving kinect sensor and sensing, deprecated
	int counter = 0;
	int angle = -5;
	*/

	//boolean value for determining when to take a picture
	bool noMovementDetected = true;
	NUI_LOCKED_RECT rect1;
	NUI_LOCKED_RECT rect2 = getLockedRect();
	int * array1 = new int[640*480*4];
	int * array2 = new int [640*480*4];


	int difsum ;
	bool isFirst = true;

	while (true)
	{
		difsum = 0;
	while (noMovementDetected)
	
	{
		sensor->NuiSkeletonGetNextFrame(0, &skelFrame);

		rect1 = getLockedRect();

		for(int kl = 0; kl < 640*480*4; kl++)
		{
			array1[kl] = rect1.pBits[kl];

		}
		rect2 = getLockedRect();
		for(int kl = 0; kl < 640*480*4; kl++)
		{
			array2[kl] = rect2.pBits[kl];

		}
			
		
		for(int i = 0; i < 640*480*4;i++)
		{
			difsum += abs(array1[i]-array2[i]);
		}
		cout << difsum << endl;
		if ((difsum >= 3000000 && !(isFirst))||(skelFrame.SkeletonData[0].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[1].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[2].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[3].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[4].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[5].eTrackingState == NUI_SKELETON_TRACKED))
		{
			noMovementDetected = false;
			takePicture(rect2.pBits,"first");
		}
		else if (difsum <= 2500000)
		{

			for (int b=0;b<640*480*4;b+=4)
			{

				arrfirst[b] = (array1[b]*3)/4;
				arrfirst[b+1] = array1[b+1]/2;
				arrfirst[b+2] = (array1[b+2]/4);
				arrfirst[b+3] = array1[b+3];
				arrfst[b] = array1[b];
				arrfst[b+1] = array1[b+1];
				arrfst[b+2] = array1[b+2];
				arrfst[b+3] = array1[b+3];

			}
		}


		difsum = 0;
		isFirst = false;
		

		/*if (counter == 1000000)
		{
			sensor->NuiCameraElevationSetAngle(angle);
			counter = 0;
			angle = -angle;
		}
		counter++;
		*/
	}
	






	  // This example assumes that the application provides
  // void* cameraFrameBuffer, a buffer for an image, and that there is a method
  // to fill the buffer with data from a camera, for example
  // cameraObj.ProcessIO(cameraFrameBuffer)

  // Create an instance of face tracker
  IFTFaceTracker* pFT = FTCreateFaceTracker();
  if(!pFT)
  {
    // Handle errors
  }

  FT_CAMERA_CONFIG myColorCameraConfig = {640, 480, NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS}; // width, height, focal length
  FT_CAMERA_CONFIG myDepthCameraConfig = {320, 240, NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS}; // width, height

  HRESULT hr = pFT->Initialize(&myColorCameraConfig, &myDepthCameraConfig, NULL, NULL);
  if( FAILED(hr) )
  {
    cout << "Failed to initialize face detection shiz" << endl;
  }

  // Create IFTResult to hold a face tracking result
  IFTResult* pFTResult = NULL;
  hr = pFT->CreateFTResult(&pFTResult);
  if(FAILED(hr))
  {
    cout << "Failed to create result shiz" << endl;
  }

  // prepare Image and SensorData for 640x480 RGB images
  pColorFrame = FTCreateImage();
  if(!pColorFrame)
  {
    cout << "Color frame failed" << endl;
  }
  pDepthFrame = FTCreateImage();
  if (!pDepthFrame)
  {
	  cout << "Depth frame failed" << endl;
  }

  // Attach assumes that the camera code provided by the application
  // is filling the buffer cameraFrameBuffer
  pColorFrame->Allocate(640, 480,  FTIMAGEFORMAT_UINT8_B8G8R8X8);
  pDepthFrame->Allocate(320,240,  FTIMAGEFORMAT_UINT16_D13P3);
  FT_SENSOR_DATA sensorData;
  sensorData.pVideoFrame = pColorFrame;
  sensorData.pDepthFrame = pDepthFrame;
  sensorData.ZoomFactor = 1.0f;
  sensorData.ViewOffset = POINT();

  bool isTracked = false;
  int counter = 0;
  int stopCounter = 0;
  float prevWeight = 0;
  // Track a face
  while (!noMovementDetected)
  {
    // Call your camera method to process IO and fill the camera buffer
		processColor();
		processDepth();
	
		
				UINT count = 1;
				UINT *cnt = &count;
				hr = pFT->DetectFaces(&sensorData, NULL, pFaces, cnt);
				if(FAILED(hr) || FAILED(pFTResult))
				{
					cout << "shiz failed " << hr << endl;
					cout << E_INVALIDARG << endl;
					cout << E_POINTER << endl;
				}
				else
				{
					if (pFaces->Weight >= 1 && pFaces->Weight != prevWeight)
					{
					takePicture(*pFaces);
					}
					prevWeight = pFaces->Weight;
				}
			 
		if (counter == 10)
		{
			counter = 0;
			takePicture(getLockedRect().pBits, "time");
		}
		rect1 = getLockedRect();

		for(int kl = 0; kl < 640*480*4; kl++)
		{
			array1[kl] = rect1.pBits[kl];

		}
		rect2 = getLockedRect();
		for(int kl = 0; kl < 640*480*4; kl++)
		{
			array2[kl] = rect2.pBits[kl];

		}
			

		int i;
		for(i = 0; i < 640*480*4;i++)
		{
			difsum += abs(array1[i]-array2[i]);
		}
		cout << "Motion Detected! " << difsum << endl;
		if (difsum <= 2500000)
		{
			stopCounter++;
			
		}
		else if (difsum >= 300000)
		{
			stopCounter--;
		}
		if (stopCounter >= 25)
		{
			noMovementDetected = true;
			rect1 = getLockedRect();
			for(int kl = 0; kl < 640*480*4; kl+=4)
			{

				
				//for blue
				//if (kl%4==0)
				//{
				//	arrfirst[kl]+=50;
				//}
				//for green
				//if (kl%4==1)
				//{
				//	arrfirst[kl]+=50;
				//}
				
				arrlast[kl] = rect1.pBits[kl]/4;
				arrlast[kl+1] = rect1.pBits[kl+1]/2;
				arrlast[kl+2] = (rect1.pBits[kl+2]*3)/4;
				arrlast[kl+3] = rect1.pBits[kl+3];
				arrlst[kl] = rect1.pBits[kl];
				arrlst[kl+1] = arrlst[kl+1];
				arrlst[kl+2] = arrlst[kl+2];
				arrlst[kl+3] = arrlst[kl+3];
			}
			takePicture(rect1.pBits,"last");
			
		}
		difsum = 0;
	  	counter++;


  }
  /*
  // Clean up.
  pFTResult->Release();
  pColorFrame->Release();
  pDepthFrame->Release();
  pFT->Release();
  */
  cout << "Send e-mail here possibly to inform owner" << endl;
  cout << "Send e-mail here possibly to inform owner" << endl;
  cout << "Send e-mail here possibly to inform owner" << endl;
  cout << "Send e-mail here possibly to inform owner" << endl;
  cout << "Send e-mail here possibly to inform owner" << endl;
  cout << "Send e-mail here possibly to inform owner" << endl;

  }
	
	return 0;
}

void processColor()
{
	NUI_IMAGE_FRAME imgFrame;
			bool frameNull = true;
		
			HRESULT hrr; 
			while (frameNull)
			{
				hrr = sensor->NuiImageStreamGetNextFrame(colorHandle, 0, &imgFrame);
				if (FAILED(hrr))
				{
					//cout << "ERROR! : " << hrr << endl;
					//cout << "S_FALSE: " << S_FALSE << endl;
					//cout << "E_NUI_FRAME_NO_DATA: " << E_NUI_FRAME_NO_DATA << endl;
					//cout << "E_POINTER: " << E_POINTER << endl;
					
				}
				else
				{

					frameNull = false;
				}
			}
		INuiFrameTexture * txture = imgFrame.pFrameTexture;
		NUI_LOCKED_RECT lockedRect;
		txture->LockRect(0,&lockedRect,NULL,0);
		memcpy(pColorFrame->GetBuffer(), PBYTE(lockedRect.pBits), min(pColorFrame->GetBufferSize(), UINT(txture->BufferLen())));
		sensor->NuiImageStreamReleaseFrame(colorHandle, &imgFrame );
		
		//txture->UnlockRect(0);

}

void processDepth()
{
	NUI_IMAGE_FRAME imgFrame;
			bool frameNull = true;
		
			HRESULT hrr; 
			while (frameNull)
			{
				hrr = sensor->NuiImageStreamGetNextFrame(depthHandle, 0, &imgFrame);
				if (FAILED(hrr))
				{
					//cout << "ERROR! : " << hrr << endl;
					//cout << "S_FALSE: " << S_FALSE << endl;
					//cout << "E_NUI_FRAME_NO_DATA: " << E_NUI_FRAME_NO_DATA << endl;
					//cout << "E_POINTER: " << E_POINTER << endl;
					
				}
				else
				{

					frameNull = false;
				}
			}
		INuiFrameTexture * txture = imgFrame.pFrameTexture;
		NUI_LOCKED_RECT lockedRect;
		txture->LockRect(0,&lockedRect,NULL,0);
		memcpy(pDepthFrame->GetBuffer(), PBYTE(lockedRect.pBits), min(pDepthFrame->GetBufferSize(), UINT(txture->BufferLen())));
		sensor->NuiImageStreamReleaseFrame(depthHandle, &imgFrame );
		
		//txture->UnlockRect(0);

}

NUI_LOCKED_RECT getLockedRect()
{
		NUI_IMAGE_FRAME imgFrame;
			bool frameNull = true;
		
			HRESULT hrr; 
			while (frameNull)
			{
				hrr = sensor->NuiImageStreamGetNextFrame(colorHandle, 0, &imgFrame);
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
		sensor->NuiImageStreamReleaseFrame(colorHandle, &imgFrame );

		//txture->UnlockRect(0);
		return lockedRect;

}

//return cluster array

vector<vector<int>> getClusters(bool* arr)
{
	vector<vector<int>> clustArr;
	//bool isFound = false;

	for(int a = 0; a<(640*480);a++)
	{
		//is the point a possible cluster?
		if (arr[a])
		{
			//has the point been added already?
			/*
			for(int b = 0; b < clustArr.size();b++)
			{

					if (std::find(clustArr[b].begin(), clustArr[b].end(), a)!=clustArr[b].end())
					{
						isFound = true;
						break;
					}

			}
			if (!isFound)
			{
				*/
				//start a new cluster
				vector<int> clust;
				populateClust(clust,arr,a);
				
				//is the cluster large enough?
				if (clust.size()>= 10)
				{
				clustArr.push_back(clust);
				}
			//}
		}
	}
	return clustArr;

}

void populateClust(vector<int> cls, bool * ar, int seed)
{

	ar[seed] = false;
	cls.push_back(seed);
	if (ar[seed-1]) populateClust(cls, ar, seed-1);
	if (ar[seed+1]) populateClust(cls, ar, seed+1);
	if (ar[seed-640]) populateClust(cls, ar, seed-640);
	if (ar[seed+640]) populateClust(cls, ar, seed+640);
}




void takePicture(FT_WEIGHTED_RECT rec)
{
	NUI_LOCKED_RECT lock = getLockedRect();
	//top line
	for (int i = 4*(rec.Rect.top * 640 + rec.Rect.left);i<=4*(rec.Rect.top * 640 + rec.Rect.right);i+=4)
	{
		lock.pBits[i] = 0;
		lock.pBits[i+1] = 0;
		lock.pBits[i+2] = 1000;
		lock.pBits[i+3] = 0;
	}
	//bottom line
	for (int i = 4*(rec.Rect.bottom * 640 + rec.Rect.left);i<=4*(rec.Rect.bottom * 640 + rec.Rect.right);i+=4)
	{
		lock.pBits[i] = 0;
		lock.pBits[i+1] = 0;
		lock.pBits[i+2] = 1000;
		lock.pBits[i+3] = 0;
	}
	//left line
	for (int i = 4*(rec.Rect.top * 640 + rec.Rect.left);i<=4*(rec.Rect.bottom * 640 + rec.Rect.left);i+=(640*4))
	{
		//blue
		lock.pBits[i] = 0;
		//green
		lock.pBits[i+1] = 0;
		//red
		lock.pBits[i+2] = 1000;
		//black/brightness?
		lock.pBits[i+3] = 0;
	}
	//right line
	for (int i = 4*(rec.Rect.top * 640 + rec.Rect.right);i<=4*(rec.Rect.bottom * 640 + rec.Rect.right);i+=(640*4))
	{
		
		lock.pBits[i] = 0;
		lock.pBits[i+1] = 0;
		lock.pBits[i+2] = 1000;
		lock.pBits[i+3] = 0;
	}
	takePicture(lock.pBits, "face");
}

void takePicture(byte * pBits, string str)
{
		if (str == "last")
		{
			for(int a = 0; a<=640*480*4;a++)
			{
				
				if ((abs(arrfst[a]-arrlst[a])+abs(arrfst[a+1]-arrlst[a+1])+abs(arrfst[a+2]-arrlst[a+2])+abs(arrfst[a+3]-arrlst[a+3])) >= 25)
				{
					arrdiff[a] = true;
				}
				
				pBits[a] = (int)(arrfirst[a]+arrlast[a]);
			}
			vector<vector<int>> clusters = getClusters(arrdiff);
			for(int b = 0; b < clusters.size();b++)
			{
				for (int c = 0; c < clusters[b].size(); c++)
				{
					pBits[clusters[b][c]*4] = 255;
					pBits[clusters[b][c]*4+1] = 0;
					pBits[clusters[b][c]*4+2] = 0;
				}
			}
		}
		//char* filename = filename = "test.bmp";
	    //FILE *pFile = fopen(filename, "wb");
		BITMAPINFOHEADER BMIH = {0};
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
        //Is this number okay?
		BMIH.biBitCount = 32;
        BMIH.biPlanes = 1;
        BMIH.biCompression = BI_RGB;
		//1280, 960
        BMIH.biWidth = 640;
        BMIH.biHeight = -480;
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
		int Number = GetTickCount()/10;       // number to be converted to a string

		string Result;          // string which will contain the result
	
		ostringstream convert;   // stream used for the conversion

		convert << Number;      // insert the textual representation of 'Number' in the characters in the stream

		Result = convert.str();
		string s = string("Pics/Pic")+Result+str+string(".jpg");
		std::wstring stemp = std::wstring(s.begin(), s.end());
		LPCWSTR sw = stemp.c_str();
		LPCWSTR a = stemp.c_str();

        HANDLE hFile = CreateFileW(a, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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
    if ( !WriteFile(hFile, static_cast<BYTE *>(pBits), BMIH.biSizeImage, &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        cout << E_FAIL << endl;
    }    

	

		bool bError = false;
		
		/*
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
  		mail.SetSenderName("Kinect Alert");
  		mail.SetSenderMail("testthiscodeplz@gmail.com");
  		mail.SetReplyTo("");
  		mail.SetSubject("Movement Detected");
  		mail.AddRecipient("Kevintasta@gmail.com");
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		mail.SetXMailer("The Bat! (v3.02) Professional");
  		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("Someone has been trying to steal your things.");
		mail.AddMsgLine("A picture has been attached for your convenience.");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("Kinect Security System.");

		
		const char * c = s.c_str();

		cout << c << endl;

		
  		mail.AddAttachment(c);
  		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
	}
	catch(ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		bError = true;
		while (true)
		{

		}
	}
	if(!bError)
		std::cout << "Mail was sent successfully.\n";
	*/
	
}

