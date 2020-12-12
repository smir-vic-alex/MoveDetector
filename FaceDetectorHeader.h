#pragma once
#ifndef FACE_DETECTOR
#define FACE_DETECTOR
#include "mainHeader.h"

extern cv::Mat grayscaleFrame; //������� �/� �����
extern const char* FRONT_CASCADE_NAME ; 
extern const char* PROFILE_CASCADE_NAME ;
extern const char* EYE_CASCADE_NAME;
extern cv::CascadeClassifier face_cascade; //������ ��� ����������� ����
extern cv::CascadeClassifier eye_cascade; //������ ��� ����������� ����
extern cv::CascadeClassifier faceProfile_cascade; //������ ��� ����������� ���� � �������
extern clock_t cl ; 
extern clock_t clClose;
extern boolean monitorIsOFF;
void loadCascades();
void log(const char *message);
Rect detectFace(Mat &grayscaleFrame, Mat &captureFrame );


#endif