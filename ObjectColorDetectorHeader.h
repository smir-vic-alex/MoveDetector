#pragma once
#ifndef COLOR_DETECTOR
#define COLOR_DETECTOR

#include "mainHeader.h"

extern Mat depthMap;
extern cv::VideoCapture cap;
extern Mat bgrImage;
extern boolean notInitControlPanel;
extern boolean notInitScanModePanel;

void detectColorObject(Mat& bgrImage, Mat& depthMap, Rect& excludeROI);
void initHandColor(cv::Mat &frame);
void scanMode();
void colorControlPanel();
void destroyScanModeWindow();

#endif