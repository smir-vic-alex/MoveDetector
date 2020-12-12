#include "mainHeader.h"
#include "FaceDetectorHeader.h"
#include "ObjectColorDetectorHeader.h"
#include "InitializerHeader.h"
#include "FrameHelperHeader.h"

int wk = 0;
Rect excludedFaceROI;

int main()
{
	connectToVideo(); //������������ � ����� ��������� ������
	initControlWindows(); //������������� ���� ��������
	loadCascades(); //������������� �������� ��� �������� ����
	clockInit(); //������������� �������� ��� ����������/��������� ��������
	
	while(true)
    {
		getFrame(cap, bgrImage); //�������� ���� �� �����������
		depthMap = getFrameForObjectTracking(bgrImage); //������������ ���� ��� �������� �������
		grayscaleFrame = getGrayFrame(bgrImage); //������������ ���������� ���� ��� �������� ����
		excludedFaceROI = detectFace(grayscaleFrame, bgrImage); //����� ����, ���������� ��������
		detectColorObject(bgrImage, depthMap, excludedFaceROI); //������� �������
		
		wk = waitKey( 20 ); //������� �������
		if( wk == 27 ) //ESC
		{
			break; //����� 
		}
		else if (wk == 32 )//Space
		{
			scanMode();//������ ���������������� ������������ �����
		}
		else if (wk == 67)//C
		{
			colorControlPanel();//������ ������� ����������� �����
		}
		else if(wk == 86)//V
		{
			destroyScanModeWindow();//�������� ���� ��������������� ������������
		}
	}
	
	waitKey();
	return 0;
}