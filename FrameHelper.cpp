#include "FrameHelperHeader.h"

Mat getFrame(VideoCapture &captureDevice, Mat &captureFrame)
{
	captureDevice>>captureFrame;
	flip(captureFrame,captureFrame,1);
	return captureFrame;
}

Mat getFrameForObjectTracking(Mat &captureFrame)
{
	Mat frame;
	cvtColor(captureFrame, frame, CV_RGBA2GRAY);
    equalizeHist(frame, frame);
	
	return frame;
}

//Получить ч/б кадр
Mat getGrayFrame(Mat &captureFrame)
{
	Mat grayFrame;
	cvtColor(captureFrame, grayFrame, CV_RGBA2GRAY);
    equalizeHist(grayFrame, grayFrame);

	return grayFrame;
}