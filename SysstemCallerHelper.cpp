#include "SysstemCallerHelperHeader.h"
#include "FaceDetectorHeader.h"
#include "ObjectColorDetectorHeader.h"

double oldX = 0;
double oldY = 0;

void offMonitorIfNeed()
{
	if(!monitorIsOFF && clock()-clClose>15000)
	{
		log("monitor Off \n");
		SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
		monitorIsOFF = true;
		clClose = 0;
	}
	cl = clock();
}

void onMonitorIfNeed()
{
	if(monitorIsOFF && clock()-cl>500)
	{
		log("monitor On \n");
		SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
		monitorIsOFF = false;
		cl = 0;
	}
	clClose = clock();
}

void moveCursor(double x, double y)
{
	if(!notInitControlPanel || !notInitScanModePanel)
	{
		return;
	}
	if(abs(x-oldX) < 15)
	{
		x = oldX;
	}
	if(abs(y-oldY) < 15)
	{
		y = oldY;
	}
	
	if(x != 0 && y != 0)
	{
		SetCursorPos(x ,y);
	}
}