// Security.cpp : Defines the entry point for the console application.
//
#include "CSmtp.h"
#include <iostream>
//#include "stdafx.h"
//#include <fstream>
#include <Windows.h>
#include <NuiApi.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include "FaceTrackLib.h"

using namespace std;


vector<vector<int>> getClusters(vector<bool> arr);
//void populateClust(vector<int> cls, bool * ar, int seed);
//method that takes a picture and e-mails it
void takePicture(FT_WEIGHTED_RECT);
void takePicture(byte * pBits, string str);
void sendPicture(string filename);
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

struct Rect {
int top;
int bot;
int left;
int right;
};

vector<double> arrfirst;

vector<int> arrfst;
vector<int> arrlst;
vector<bool> arrdiff;
vector<Rect> arrrect;




Rect getRectangle(vector<int> cluster);

int resWidth;
int resHeight;

/*
double arrfirst[resWidth*resHeight*4];
double arrlast[resWidth*resHeight*4];
int arrfst[resWidth*resHeight*4];
int arrlst[resWidth*resHeight*4];
bool arrdiff[resWidth*resHeight];
*/

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


	resWidth = 640;
	resHeight = 480;
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
	int * array1 = new int[resWidth*resHeight*4];
	int * array2 = new int[resWidth*resHeight*4];

	arrfirst.resize(resWidth*resHeight*4);
	arrfst.resize(resWidth*resHeight*4);
	arrlst.resize(resWidth*resHeight*4);
	arrdiff.resize(resWidth*resHeight);
	//array1.resize(resWidth*resHeight*4);
	//array2.resize(resWidth*resHeight*4);


	int difsum ;
	int defControl;
	bool isFirst = true;

	IFTFaceTracker* pFT = FTCreateFaceTracker();
  if(!pFT)
  {
    // Handle errors
  }

  FT_CAMERA_CONFIG myColorCameraConfig = {resWidth, resHeight, NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS}; // width, height, focal length
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

  // prepare Image and SensorData for resWidthxresHeight RGB images
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
  pColorFrame->Allocate(resWidth, resHeight,  FTIMAGEFORMAT_UINT8_B8G8R8X8);
  pDepthFrame->Allocate(320,240,  FTIMAGEFORMAT_UINT16_D13P3);
  FT_SENSOR_DATA sensorData;

	while (true)
	{
		difsum = 0;
		defControl = 5;
		arrrect.clear();
	while (noMovementDetected)
	
	{
		sensor->NuiSkeletonGetNextFrame(0, &skelFrame);

		rect1 = getLockedRect();

		for(int kl = 0; kl < resWidth*resHeight*4; kl++)
		{
			array1[kl] = rect1.pBits[kl];

		}
		rect2 = getLockedRect();
		for(int kl = 0; kl < resWidth*resHeight*4; kl++)
		{
			array2[kl] = rect2.pBits[kl];

		}
			
		
		for(int i = 0; i < resWidth*resHeight*4;i++)
		{
			difsum += abs(array1[i]-array2[i]);
		}
		cout << difsum << endl;
		if ((difsum >= 2600000 && !(isFirst))||(skelFrame.SkeletonData[0].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[1].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[2].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[3].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[4].eTrackingState == NUI_SKELETON_TRACKED||skelFrame.SkeletonData[5].eTrackingState == NUI_SKELETON_TRACKED))
		{
			noMovementDetected = false;
			takePicture(rect2.pBits,"first");
		}
		else if (difsum <= 2400000)
		{
			if (defControl >=5)
			{
				for (int b=0;b<resWidth*resHeight*4;b+=4)
				{

					arrfirst[b] = (array1[b]*3.0)/4.0;
					arrfirst[b+1] = array1[b+1]/2.0;
					arrfirst[b+2] = (array1[b+2]/4.0);
					arrfirst[b+3] = array1[b+3]/2.0;
					arrfst[b] = array1[b];
					arrfst[b+1] = array1[b+1];
					arrfst[b+2] = array1[b+2];
					arrfst[b+3] = array1[b+3];

				}
				defControl=0;
			}
			defControl++;
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
					if (pFaces->Weight >= 4 && pFaces->Weight != prevWeight)
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

		for(int kl = 0; kl < resWidth*resHeight*4; kl++)
		{
			array1[kl] = rect1.pBits[kl];

		}
		rect2 = getLockedRect();
		for(int kl = 0; kl < resWidth*resHeight*4; kl++)
		{
			array2[kl] = rect2.pBits[kl];

		}
			

		int i;
		for(i = 0; i < resWidth*resHeight*4;i++)
		{
			difsum += abs(array1[i]-array2[i]);
		}
		cout << "Motion Detected! " << difsum << endl;
		if (difsum <= 2400000)
		{
			stopCounter++;
			
		}
		else if (difsum >= 2400000 && stopCounter >= -30)
		{
			int amnt = ((difsum-2400000)/100000);
			if (amnt >= 15) amnt = 15;
			stopCounter-= amnt;
		}
		if (stopCounter >= 50)
		{
			noMovementDetected = true;
			rect1 = getLockedRect();
			takePicture(rect1.pBits, "last");
			byte * bits = new byte[resWidth*resHeight*4];
			for(int kl = 0; kl < resWidth*resHeight*4; kl+=4)
			{
	
				bits[kl] = (int) (rect1.pBits[kl]/4.0 + arrfirst[kl]);
				bits[kl+1] = (int) (rect1.pBits[kl+1]/2.0 + arrfirst[kl+1]);
				bits[kl+2] = (int) ((rect1.pBits[kl+2]*3.0)/4 + arrfirst[kl+2]);
				bits[kl+3] = (int) (rect1.pBits[kl+3]/2.0 + arrfirst[kl+3]);
				arrlst[kl] = rect1.pBits[kl];
				arrlst[kl+1] = rect1.pBits[kl+1];
				arrlst[kl+2] = rect1.pBits[kl+2];
				arrlst[kl+3] = rect1.pBits[kl+3];

			}
			takePicture(bits, "noRect");
			takePicture(bits,"final");
			delete bits;			
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

vector<vector<int>> getClusters(vector<bool> arr)
{
	vector<vector<int>> clustArr;
	//bool isFound = false;
	bool isMerged = false;
	int clusToMerge;

	for(int a = 0; a<(resWidth*resHeight);a++)
	{
		isMerged = false;
		clusToMerge = -1;
		if (a%10000==0) cout << "Working... " << a << endl;
		//is the point a possible cluster?
		if (arr[a])
		{
			//cout << "Current array size:" << clustArr.size() << endl;
			
				
				//are there already defined clusters around it? If so, add it to cluster
				if (a>0 && arr[a-1])
				{
					for(int b = 0; b < clustArr.size();b++)
					{

						if (std::find(clustArr[b].begin(), clustArr[b].end(), a-1)!=clustArr[b].end())
						{
							clustArr[b].push_back(a);
							isMerged = true;
							clusToMerge = b;
							break;
						}

					}
				}
				if (a>=resWidth && arr[a-resWidth])
				{
					for(int b = 0; b < clustArr.size();b++)
					{

						if (std::find(clustArr[b].begin(), clustArr[b].end(), a-resWidth)!=clustArr[b].end() && b != clusToMerge)
						{
							//has this point already been merged? If so, merge existing clusters
							
							if (isMerged)
							{
								//cout << "Things are being merged!" << endl;
								clustArr[b].insert( clustArr[b].end(), clustArr[clusToMerge].begin(), clustArr[clusToMerge].end() );
								clustArr.erase(clustArr.begin() + clusToMerge);								
								if (clusToMerge < b) b--;
							}
							else
							{
								clustArr[b].push_back(a);
								isMerged = true;
							}
							break;
						}
					}
				}
				//if no clusters nearby, start new cluster
				if (!isMerged)
				{
					vector<int> newClust;
					newClust.push_back(a);
					clustArr.push_back(newClust);
				}
				
			//}
		}
	}
	//remove any clusters that are too small
	for(int c = clustArr.size()-1; c >= 0; c--)
	{
		if (clustArr[c].size() <= 10) clustArr.erase(clustArr.begin() + c);
	}
	return clustArr;

}

Rect getRectangle(vector<int> cluster)
{
	Rect rec;
	rec.top = cluster[0];
	rec.bot = cluster[0];
	rec.left = cluster[0];
	rec.right = cluster[0];
	
	for (int i=1; i<cluster.size();i++)
	{
		if (cluster[i]<rec.top) rec.top = cluster[i];
		if (cluster[i]>rec.bot) rec.bot = cluster[i];
		if (cluster[i]%resWidth<rec.left%resWidth) rec.left = cluster[i];
		if (cluster[i]%resWidth>rec.right%resWidth) rec.right = cluster[i];
	}
	

	rec.top /= resWidth;
	rec.bot /= resWidth;
	rec.left %= resWidth;
	rec.right %= resWidth;

	rec.left-=2;
	rec.right+=2;
	rec.top-=2;
	rec.bot+=2;

	if (rec.top<0) rec.top = 1;
	if (rec.bot>resHeight-1) rec.bot = resHeight-1;
	if (rec.left<0) rec.left = 1;
	if (rec.right>resWidth-1) rec.right = resWidth-1;

	cout << rec.top << " " << rec.bot << " " << rec.left << " " << rec.right  << endl;

	//prevent rectangles that are deflated from appearing
	if(rec.right-rec.left <= 3 || rec.bot-rec.top <=3)
	{
		rec.top = 0;
		rec.bot = 0;
		rec.right = 0;
		rec.left = 0;
		return rec;
	}
	
	for(int b = 0; b < arrrect.size(); b++)
	{
		if (!(rec.left > arrrect[b].right || rec.right < arrrect[b].left || rec.top > arrrect[b].bot || rec.bot < arrrect[b].top))
		{
			if (arrrect[b].left < rec.left) rec.left = arrrect[b].left;
			if (arrrect[b].right > rec.right) rec.right = arrrect[b].right;
			if (arrrect[b].top >rec.top) rec.top = arrrect[b].top;
			if (arrrect[b].bot < rec.bot) rec.bot = arrrect[b].bot;
			arrrect.erase(arrrect.begin() + b);
		}
	}
	
	return rec;
	
}

/*
void populateClust(vector<int> cls, bool * ar, int seed)
{

	ar[seed] = false;
	cls.push_back(seed);
	if (seed%resWidth!=0 && ar[seed-1]) populateClust(cls, ar, seed-1);
	if (seed%resWidth!=639 && ar[seed+1]) populateClust(cls, ar, seed+1);
	if (seed >= resWidth && ar[seed-resWidth]) populateClust(cls, ar, seed-resWidth);
	if (seed < 639*resHeight && ar[seed+resWidth]) populateClust(cls, ar, seed+resWidth);
	cout << "plants" << endl;
}
*/



void takePicture(FT_WEIGHTED_RECT rec)
{
	NUI_LOCKED_RECT lock = getLockedRect();
	//top line
	for (int i = 4*(rec.Rect.top * resWidth + rec.Rect.left);i<=4*(rec.Rect.top * resWidth + rec.Rect.right);i+=4)
	{
		lock.pBits[i] = 0;
		lock.pBits[i+1] = 0;
		lock.pBits[i+2] = 254;
		lock.pBits[i+3] = 0;
	}
	//bottom line
	for (int i = 4*(rec.Rect.bottom * resWidth + rec.Rect.left);i<=4*(rec.Rect.bottom * resWidth + rec.Rect.right);i+=4)
	{
		lock.pBits[i] = 0;
		lock.pBits[i+1] = 0;
		lock.pBits[i+2] = 254;
		lock.pBits[i+3] = 0;
	}
	//left line
	for (int i = 4*(rec.Rect.top * resWidth + rec.Rect.left);i<=4*(rec.Rect.bottom * resWidth + rec.Rect.left);i+=(resWidth*4))
	{
		//blue
		lock.pBits[i] = 0;
		//green
		lock.pBits[i+1] = 0;
		//red
		lock.pBits[i+2] = 254;
		//alpha
		lock.pBits[i+3] = 0;
	}
	//right line
	for (int i = 4*(rec.Rect.top * resWidth + rec.Rect.right);i<=4*(rec.Rect.bottom * resWidth + rec.Rect.right);i+=(resWidth*4))
	{
		
		lock.pBits[i] = 0;
		lock.pBits[i+1] = 0;
		lock.pBits[i+2] = 254;
		lock.pBits[i+3] = 0;
	}
	takePicture(lock.pBits, "face");
}

void takePicture(byte * pBits, string str)
{
		if (str == "final")
		{
			for(int a = 0; a<resWidth*resHeight*4;a+=4)
			{
				
					if ((abs(arrfst[a]-arrlst[a])+abs(arrfst[a+1]-arrlst[a+1])+abs(arrfst[a+2]-arrlst[a+2])+abs(arrfst[a+3]-arrlst[a+3])) >= 40)
					{
						//cout << "Things are true"  << endl;
						arrdiff[a/4] = true;
					}
					else
					{
						arrdiff[a/4] = false;
					}
				
				
			}

			
			
			vector<vector<int>> clusters = getClusters(arrdiff);
			cout << "Number of clusters: " << clusters.size() << endl;
			
			for(int b = 0; b < clusters.size();b++)
			{
				cout << "Cluster size: " << clusters[b].size() << endl;
				arrrect.push_back(getRectangle(clusters[b]));
				
			}
			for(int b = 0; b < arrrect.size();b++)
			{
				Rect rec = arrrect[b];
				for (int i = 4*(rec.top * resWidth + rec.left);i<=4*(rec.top * resWidth + rec.right);i+=4)
				{
					pBits[i] = 0;
					pBits[i+1] = 0;
					pBits[i+2] = 254;
					pBits[i+3] = 0;
				}
				//bottom line
				for (int i = 4*(rec.bot * resWidth + rec.left);i<=4*(rec.bot * resWidth + rec.right);i+=4)
				{
					pBits[i] = 0;
					pBits[i+1] = 0;
					pBits[i+2] = 254;
					pBits[i+3] = 0;
				}
				//left line
				for (int i = 4*(rec.top * resWidth + rec.left);i<=4*(rec.bot * resWidth + rec.left);i+=(resWidth*4))
				{
					//blue
					pBits[i] = 0;
					//green
					pBits[i+1] = 0;
					//red
					pBits[i+2] = 254;
					//alpha
					pBits[i+3] = 0;
				}
				//right line
				for (int i = 4*(rec.top * resWidth + rec.right);i<=4*(rec.bot * resWidth + rec.right);i+=(resWidth*4))
				{
		
					pBits[i] = 0;
					pBits[i+1] = 0;
					pBits[i+2] = 254;
					pBits[i+3] = 0;
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
        BMIH.biWidth = resWidth;
        BMIH.biHeight = -resHeight;
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
    if ( !WriteFile(hFile, static_cast <BYTE*> ( pBits), BMIH.biSizeImage, &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        cout << E_FAIL << endl;
    }    

	

		bool bError = false;
		
		
	if (str == "final")
	{
		sendPicture(s);
	}
}

void sendPicture(string filename)
{
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

		
		const char * c = filename.c_str();

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
	
}

