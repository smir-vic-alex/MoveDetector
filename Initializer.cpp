#include "InitializerHeader.h"
#include "FaceDetectorHeader.h"
#include "ObjectColorDetectorHeader.h"

HWND hwnd = nullptr;


int initVideo(HWND &hwnd, VideoCapture &captureDevice)
{
	hwnd = GetDesktopWindow();
	printf("\nCheck access to camera ...");
	if(!captureDevice.open(0))
	{
		printf("\nNot access to camera \n");
		return 0;
	}
	
	printf(" success \n");
	//namedWindow("outputCapture", 1);
	return 1;
}

void initControlWindows()
{
	//namedWindow("depth", 1);
	namedWindow("bgr", 1);
}

int connectToVideo()
{
	if(!initVideo(hwnd, cap)) //Инициализация видео с веб камеры
	{
		printf("\nNo find camera\n");
		printf("\nExit with code -1 \n");
		system ("pause");
		return -1;	
	}
}

int loadCascade(CascadeClassifier &cascade,const String &name)
{
	try
	{
		cascade.load(name);
		if(cascade.empty())
		{
			return 0;
		}
	}
	catch(Exception e)
	{
		printf(e.what()); 
		system ("pause"); 
	}
	return 1;
}

int cascadeLoadError()
{
	printf("\nNo find cascades\n");
	printf("\nExit with code -1 \n");
	system ("pause");
	return -1;
}

void loadCascades()
{
	if(!loadCascade(face_cascade, FRONT_CASCADE_NAME) 
		&& !loadCascade(faceProfile_cascade, PROFILE_CASCADE_NAME) 
		&& !loadCascade(eye_cascade, EYE_CASCADE_NAME)) //Загрузка каскада Хаара 
	{
		cascadeLoadError();
	}
}

void clockInit()
{
	cl = clock();
	clClose = clock();
}