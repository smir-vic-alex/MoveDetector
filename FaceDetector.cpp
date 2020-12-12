#include "FaceDetectorHeader.h"
#include "SysstemCallerHelperHeader.h"

const char* FRONT_CASCADE_NAME = "haarcascade_frontalface_alt_tree.xml"; 
const char* PROFILE_CASCADE_NAME = "haarcascade_profileface.xml";
const char* EYE_CASCADE_NAME = "haarcascade_eye.xml";
//HWND hwnd = nullptr;
CascadeClassifier face_cascade; //Каскад для определения лица
CascadeClassifier eye_cascade; //Каскад для определения глаз
CascadeClassifier faceProfile_cascade; //Каскад для определения лица в профиль

VideoCapture captureDevice; //Камера

vector<Rect> faces; //Массив найденных лиц
vector<Rect> eyes; //Массив найденных глаз

Mat captureFrame; //Матрица цветного кадра
Mat grayscaleFrame;
Mat faceTemplate; //Матрица шаблона
Mat m_matchingResult; //Матрица шаблона
Mat r ; //Матрица поворота

Rect face; //Область лица
Rect faceROI; //"Область интересов" вокруг области лица
Rect eyesROI; //"Область интересов" вокруг области глаз

Point m_facePosition = NULL; //Координаты центра лица

boolean detect = false; //Флаг - нашли лицо или нет
boolean monitorIsOFF = false; //Флаг - монитор выключен
boolean foundAngles = false; //Флаг - найдены углы поворта
boolean logIsON = false; //Флаг - логирование включено
bool m_templateMatchingRunning = false; //Флаг - поиск по шаблону

//Число мс для нахождения по шаблону 
int64 m_templateMatchingStartTime = 0; 
int64 m_templateMatchingCurrentTime = 0;

double m_templateMatchingMaxDuration = 3; //Максимальное время поиска по шаблону 
double m_scale = 1; //Масштаб
float angle = 0; //Угол поворота головы

//Таймеры
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
	if(foundAngles) //Если были найдены углы поворота изображения в области лица на предыдущем шаге цикла, то поворочиваем изображение
		warpAffine(grayscaleFrame(faceROI), grayscaleFrame(faceROI), r, Size(grayscaleFrame(faceROI).cols, grayscaleFrame(faceROI).rows)); //Поворот области лица на всем кадре на найденную в предыдущем шаге цикла матрицу поворота
					
	if(!detect) //Если лицо не найдено
	{
		detectFaces(grayscaleFrame); //Пытаемся найти лицо во всем кадре
		foundAngles = false; //Снимаем флаг того, что есть углы поворота лица по глазам
		offMonitorIfNeed(); //Если на протяжении заданного времени лицо так и небыло найдено, то выключаем монитор
	}
	else
	{
		onMonitorIfNeed(); //Если на протяжении заданного времени лицо стабильно находилось, то включаем монитор
		detectFaceAroundRoi(grayscaleFrame); //Находим лицо не во всем кадре, а только в небольшой области, где предположительно оно находится
		detectEyes(grayscaleFrame); //Находим глаза и определяем угол наклона головы
		if(foundAngles) //Если нашли угол, то пытаемся снова найти лицо, но с уже повернутой областью лица
			detectFaceAroundRoi(grayscaleFrame);
				
		if (m_templateMatchingRunning) //Флаг того что  не нашли лицо в заданной области, попытаемся найти его по заранее сохраненному шаблону
		{
			log("try detect using template matching \n");
			detectFaceTemplateMatching(grayscaleFrame); //Пытаемся найти лицо по заранее сохраненному шаблону
		}
	}
		
	//drawRectagle(grayscaleFrame, rectangleFace()); //Обводим область лица на ч/б кадре прямоугольником
	Rect faceRect = rectangleFace();
	drawRectagle(captureFrame, faceRect); //Обводим область лица на цветном кадре прямоугольником
	//circle(captureFrame, m_facePosition, 30, Scalar(0, 255, 0)); //Либо обводим окружностью
		
	//imshow("grayscale", grayscaleFrame); //Покажем ч/б кадр
	//imshow("outputCapture", captureFrame); //Покажем цветной кадр
	
    return faceRect;
}
