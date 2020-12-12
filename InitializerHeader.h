#pragma once
#ifndef INITIALIZER
#define INITIALIZER

#include "mainHeader.h"
extern HWND hwnd;
int initVideo(HWND &hwnd, cv::VideoCapture &captureDevice);
int connectToVideo();
void initControlWindows();
void loadCascades();
void clockInit();

#endif