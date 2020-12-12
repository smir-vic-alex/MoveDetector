#include "FaceDetectorHeader.h"
#include "SysstemCallerHelperHeader.h"

const char* FRONT_CASCADE_NAME = "haarcascade_frontalface_alt_tree.xml"; 
const char* PROFILE_CASCADE_NAME = "haarcascade_profileface.xml";
const char* EYE_CASCADE_NAME = "haarcascade_eye.xml";
//HWND hwnd = nullptr;
CascadeClassifier face_cascade; //������ ��� ����������� ����
CascadeClassifier eye_cascade; //������ ��� ����������� ����
CascadeClassifier faceProfile_cascade; //������ ��� ����������� ���� � �������

VideoCapture captureDevice; //������

vector<Rect> faces; //������ ��������� ���
vector<Rect> eyes; //������ ��������� ����

Mat captureFrame; //������� �������� �����
Mat grayscaleFrame;
Mat faceTemplate; //������� �������
Mat m_matchingResult; //������� �������
Mat r ; //������� ��������

Rect face; //������� ����
Rect faceROI; //"������� ���������" ������ ������� ����
Rect eyesROI; //"������� ���������" ������ ������� ����

Point m_facePosition = NULL; //���������� ������ ����

boolean detect = false; //���� - ����� ���� ��� ���
boolean monitorIsOFF = false; //���� - ������� ��������
boolean foundAngles = false; //���� - ������� ���� �������
boolean logIsON = false; //���� - ����������� ��������
bool m_templateMatchingRunning = false; //���� - ����� �� �������

//����� �� ��� ���������� �� ������� 
int64 m_templateMatchingStartTime = 0; 
int64 m_templateMatchingCurrentTime = 0;

double m_templateMatchingMaxDuration = 3; //������������ ����� ������ �� ������� 
double m_scale = 1; //�������
float angle = 0; //���� �������� ������

//�������
const double TICK_FREQUENCY = getTickFrequency(); 
clock_t cl = NULL ; 
clock_t clClose = NULL;

void log(const char *message)
{
	if(logIsON)
		printf(message);
}

Rect calculateROI(const Rect &inputRect, const Rect &frameSize)
{
	Rect outputRect;
	
	outputRect.width = inputRect.width * 2;
    outputRect.height = inputRect.height * 2;

    outputRect.x = inputRect.x - inputRect.width / 2;
    outputRect.y = inputRect.y - inputRect.height / 2;

	if (outputRect.x < frameSize.x) {
        outputRect.width += outputRect.x;
        outputRect.x = frameSize.x;
    }
    if (outputRect.y < frameSize.y) {
        outputRect.height += outputRect.y;
        outputRect.y = frameSize.y;
    }

    if (outputRect.x + outputRect.width > frameSize.width) {
        outputRect.width = frameSize.width - outputRect.x;
    }
    if (outputRect.y + outputRect.height > frameSize.height) {
        outputRect.height = frameSize.height - outputRect.y;
    }

    return outputRect;	
}

Point centerOfRect(const Rect &rect)
{
    return Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

Rect biggestFace(vector<Rect> &faces)
{
    assert(!faces.empty());
	Rect *biggest = &faces[0];
    for (auto &face : faces) {
        if (face.area() < biggest->area())
            biggest = &face;
    }
    return *biggest;
}

Mat getFaceTemplate(const Mat &frame, Rect face)
{
    face.x += face.width / 4;
    face.y += face.height / 4;
    face.width /= 2;
    face.height /= 2;

    Mat faceTemplate = frame(face).clone();
    return faceTemplate;
}

void detectFaces(Mat &frame)
{
	log("try detect in whole frame \n");

	face_cascade.detectMultiScale(frame, faces, 1.1, 3,0,//CV_HAAR_FIND_BIGGEST_OBJECT,
        Size(frame.rows / 5, frame.rows / 5),
        Size(frame.rows * 2 / 3, frame.rows * 2 / 3));
	
	if(faces.size()==0)
	{
		log("detect in whole frame frontal face is FAIL \n");
		
		log("try detect in whole frame profile face \n");
		
		faceProfile_cascade.detectMultiScale(frame, faces, 1.1, 3,0,//CV_HAAR_FIND_BIGGEST_OBJECT,
        Size(frame.rows / 5, frame.rows / 5),
        Size(frame.rows * 2 / 3, frame.rows * 2 / 3));
		
	}
	if(faces.size()==0)
	{
		log("detect in whole frame profile face is FAIL \n");
		return;
	}
	log("detect in whole frame is SUCCESS \n");
	detect = true;
	face = biggestFace(faces); //faces[0];
	faceTemplate = getFaceTemplate(frame, face);
	faceROI = calculateROI(face, Rect(0, 0, frame.cols, frame.rows));
	m_facePosition = centerOfRect(face);
}

void detectFaceAroundRoi(Mat &frame)
{
	log("detect around ROI");

	face_cascade.detectMultiScale(frame(faceROI), faces, 1.1, 3,0,// CV_HAAR_FIND_BIGGEST_OBJECT,
        Size(face.width * 8 / 10, face.height * 8 / 10),
        Size(face.width * 12 / 10, face.width * 12 / 10));
	
	if(faces.size()==0)
	{
		log("detect in ROI frontal face is FAIL \n");
		
		log("try detect in ROI profile face \n");
		
		faceProfile_cascade.detectMultiScale(frame(faceROI), faces, 1.1, 3,0,//CV_HAAR_FIND_BIGGEST_OBJECT,
        Size(face.width * 8 / 10, face.height * 8 / 10),
        Size(face.width * 12 / 10, face.width * 12 / 10));
	}

	if(faces.size()==0)
	{
		log("detect around ROI is FAIL \n");
		log("need try detect using matching \n");
		detect = false;
		m_templateMatchingRunning = true;
        if (m_templateMatchingStartTime == 0)
            m_templateMatchingStartTime = getTickCount();
        return;
	}
	log("detect around ROI is SUCCESS \n");
	m_templateMatchingRunning = false;
    m_templateMatchingCurrentTime = m_templateMatchingStartTime = 0;
	face = biggestFace(faces); //faces[0];
	face.x += faceROI.x;
	face.y += faceROI.y;
	faceTemplate = getFaceTemplate(frame, face);
	faceROI = calculateROI(face, Rect(0, 0, frame.cols, frame.rows));
	m_facePosition = centerOfRect(face);
}

void detectFaceTemplateMatching(Mat &frame)
{
	m_templateMatchingCurrentTime = getTickCount();
    double duration = (double)(m_templateMatchingCurrentTime - m_templateMatchingStartTime)/ TICK_FREQUENCY; 
    
    if (duration > m_templateMatchingMaxDuration) {
        detect = false;
        m_templateMatchingRunning = false;
        m_templateMatchingStartTime = m_templateMatchingCurrentTime = 0;
    }
	
	matchTemplate(frame(faceROI), faceTemplate, m_matchingResult, CV_TM_SQDIFF_NORMED);
	
	//imshow("matchTemplate", faceTemplate);
	
	double min, max;
    Point minLoc, maxLoc;
    minMaxLoc(m_matchingResult, &min, &max, &minLoc, &maxLoc);
	normalize(m_matchingResult, m_matchingResult, 0, 1, NORM_MINMAX, -1, Mat());
	
	//imshow("m_matchingResult", m_matchingResult);
	
	minLoc.x += faceROI.x;
    minLoc.y += faceROI.y;
	
	face = Rect(minLoc.x, minLoc.y, faceTemplate.cols, faceTemplate.rows);
    face = calculateROI(face, Rect(0, 0, frame.cols, frame.rows));
	faceTemplate = getFaceTemplate(frame, face);
	faceROI = calculateROI(face, Rect(0, 0, frame.cols, frame.rows));
	
	m_facePosition = centerOfRect(face);
}

void detectEyes(Mat &frame)
{
	eyesROI = calculateROI(Rect(face.x,face.y,face.width,face.height/2), Rect(0,0,frame.cols,frame.rows));
	//imshow("eyesROI", frame(eyesROI));
	log("try detect eyes.. \n");
	eye_cascade.detectMultiScale( frame(eyesROI), eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

	if(eyes.size()==2)
	{
		log("detect eyes is SUCCESS \n");
		Point ept1 (eyesROI.x + eyes[0].x + eyes[0].width*0.5,eyesROI.y + eyes[0].y + eyes[0].width*0.5) ;
		Point ept2 (eyesROI.x + eyes[1].x + eyes[1].width*0.5,eyesROI.y + eyes[1].y + eyes[1].width*0.5) ;
		if(ept1.x>ept2.x)
		{
			Point buf = ept1;
			ept1 = ept2;
			ept2 = buf;
		}
					
		float distanceX = (float) (eyes[1].x ) - (float) eyes[0].x;
		float distanceY = (float) (eyes[1].y) - (float) eyes[0].y;
		angle = atan2(distanceY, distanceX) * 180 / CV_PI ;
				
		if(-60<angle && angle<60 )
		{
			log("angle of rotate is");
			//cout<<angle<<" \n";
			Point2f pt(grayscaleFrame (faceROI).cols/2.,grayscaleFrame(faceROI).rows/2.);
			r = getRotationMatrix2D(pt, angle, 1.0);
			warpAffine(grayscaleFrame(faceROI), grayscaleFrame(faceROI), r, Size(grayscaleFrame(faceROI).cols, grayscaleFrame(faceROI).rows));
			foundAngles = true;
		}
		//line(captureFrame, ept1, ept2, cvScalar(0, 255, 0, 0), 1, 8, 0);
		line(grayscaleFrame, ept1, ept2, cvScalar(0, 255, 0, 0), 1, 8, 0);
	}
	else
	{
		log("detect eyes is FAIL \n");
		return;
	}
}

Rect rectangleFace() 
{
    Rect faceRect = face;
    faceRect.x = (int)(faceRect.x / m_scale);
    faceRect.y = (int)(faceRect.y / m_scale);
    faceRect.width = (int)(faceRect.width / m_scale);
    faceRect.height = (int)(faceRect.height / m_scale);
    return faceRect;
}

void drawRectagle(Mat &frame, Rect &rectagle)
{
	rectangle(frame, rectagle, cvScalar(0, 255, 0, 0), 1, 8, 0);
}

Rect detectFace(Mat &grayscaleFrame, Mat &captureFrame )
{
	if(foundAngles) //���� ���� ������� ���� �������� ����������� � ������� ���� �� ���������� ���� �����, �� ������������ �����������
		warpAffine(grayscaleFrame(faceROI), grayscaleFrame(faceROI), r, Size(grayscaleFrame(faceROI).cols, grayscaleFrame(faceROI).rows)); //������� ������� ���� �� ���� ����� �� ��������� � ���������� ���� ����� ������� ��������
					
	if(!detect) //���� ���� �� �������
	{
		detectFaces(grayscaleFrame); //�������� ����� ���� �� ���� �����
		foundAngles = false; //������� ���� ����, ��� ���� ���� �������� ���� �� ������
		offMonitorIfNeed(); //���� �� ���������� ��������� ������� ���� ��� � ������ �������, �� ��������� �������
	}
	else
	{
		onMonitorIfNeed(); //���� �� ���������� ��������� ������� ���� ��������� ����������, �� �������� �������
		detectFaceAroundRoi(grayscaleFrame); //������� ���� �� �� ���� �����, � ������ � ��������� �������, ��� ���������������� ��� ���������
		detectEyes(grayscaleFrame); //������� ����� � ���������� ���� ������� ������
		if(foundAngles) //���� ����� ����, �� �������� ����� ����� ����, �� � ��� ���������� �������� ����
			detectFaceAroundRoi(grayscaleFrame);
				
		if (m_templateMatchingRunning) //���� ���� ���  �� ����� ���� � �������� �������, ���������� ����� ��� �� ������� ������������ �������
		{
			log("try detect using template matching \n");
			detectFaceTemplateMatching(grayscaleFrame); //�������� ����� ���� �� ������� ������������ �������
		}
	}
		
	//drawRectagle(grayscaleFrame, rectangleFace()); //������� ������� ���� �� �/� ����� ���������������
	Rect faceRect = rectangleFace();
	drawRectagle(captureFrame, faceRect); //������� ������� ���� �� ������� ����� ���������������
	//circle(captureFrame, m_facePosition, 30, Scalar(0, 255, 0)); //���� ������� �����������
		
	//imshow("grayscale", grayscaleFrame); //������� �/� ����
	//imshow("outputCapture", captureFrame); //������� ������� ����
	
    return faceRect;
}
