#pragma once
#ifndef FRAME_HELPER
#define FRAME_HELPER

#include "mainHeader.h"

cv::Mat getFrame(VideoCapture &captureDevice,cv::Mat &captureFrame);
cv::Mat getFrameForObjectTracking(cv::Mat &captureFrame);
cv::Mat getGrayFrame(Mat &captureFrame);

#endif