#include "ObjectColorDetectorHeader.h"
#include "SysstemCallerHelperHeader.h"

Mat depthMap;
VideoCapture cap;
Mat bgrImage;
const char* CONTROL_NAME_WINDOW = "Control";
const char* SCAN_NAME_WINDOW = "Scan";
const char* DEPTH_NAME_WINDOW = "Depth";
const int DEFAULT_DELAY_BEFORE_DESTROY = 7000;
vector <Rect> roi;
Point upper_corner;
Point lower_corner;
Scalar color;
int border_thickness=2;

int iLowH = 93;
int iHighH = 113;
int iLowS = 239; 
int iHighS = 259;
int iLowV = 243;
int iHighV = 263;

boolean notInitControlPanel = true;
boolean notInitScanModePanel = true;

map < char*, int*> trackBarSettings;


int findBiggestContour(vector<vector<Point> > contours)
{
    int indexOfBiggestContour = -1;
    int sizeOfBiggestContour = 0;
    for (int i = 0; i < contours.size(); i++){
        if(contours[i].size() > sizeOfBiggestContour)
		{
            sizeOfBiggestContour = contours[i].size();
            indexOfBiggestContour = i;
        }
    }
    return indexOfBiggestContour;
}

Rect calcRect(Point u_corner, Point l_corner, Mat src)
{
	upper_corner = u_corner;
	lower_corner=l_corner;
	color=Scalar(0,255,0);
	border_thickness=2;
	return Rect(u_corner.x, u_corner.y, l_corner.x-u_corner.x,l_corner.y-u_corner.y);
}

void findAverage(Mat &frame, vector <Rect> roi)
{
	Vector<Scalar> means (roi.size()); 
	
	for(int i=0; i< roi.size(); i++)
	{
		 means[i] = mean(frame(roi[i]));
	}
	Scalar medium(0,0,0);
	for(int i=0; i< means.size(); i++)
	{
		medium[0] += means[i][0];
		medium[1] += means[i][1];
		medium[2] += means[i][2];
	}
	
	medium[0] /= means.size();
	medium[1] /= means.size();
	medium[2] /= means.size();

	int valueRange = 13;

	iLowH = (int)medium[0] - valueRange;
	iHighH = (int)medium[0] + valueRange;

	iLowS = (int)medium[1] - valueRange; 
	iHighS = (int)medium[1] + valueRange;

	iLowV = (int)medium[2] - valueRange;
	iHighV = (int)medium[2] + valueRange;

}

void initHandColor(Mat &frame)
{
	int square_len=20;

	//roi.push_back(calcRect(Point(frame.cols/3, frame.rows/6),Point(frame.cols/3+square_len,frame.rows/6+square_len),frame));
	//roi.push_back(calcRect(Point(frame.cols/4, frame.rows/2),Point(frame.cols/4+square_len,frame.rows/2+square_len),frame));
	//roi.push_back(calcRect(Point(frame.cols/3, frame.rows/1.5),Point(frame.cols/3+square_len,frame.rows/1.5+square_len),frame));
	roi.push_back(calcRect(Point(frame.cols/2, frame.rows/2),Point(frame.cols/2+square_len,frame.rows/2+square_len),frame));
	roi.push_back(calcRect(Point(frame.cols/2.5, frame.rows/2.5),Point(frame.cols/2.5+square_len,frame.rows/2.5+square_len),frame));
	roi.push_back(calcRect(Point(frame.cols/2, frame.rows/1.5),Point(frame.cols/2+square_len,frame.rows/1.5+square_len),frame));
	//roi.push_back(calcRect(Point(frame.cols/2.5, frame.rows/1.8),Point(frame.cols/2.5+square_len,frame.rows/1.8+square_len),frame));

	findAverage(frame, roi);
	
	for(int i=0; i<roi.size(); i++)
	{
		rectangle(frame, roi[i], color, border_thickness);
	}

	imshow(SCAN_NAME_WINDOW, frame);
}

void detectObj(Mat& mask, Mat& bgr)
{
	int largest_area=0;
	int largest_contour_index=0;
	Rect bounding_rect;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat frm ;
	Rect roi;
	roi.x = 0;
	roi.y = 0;
	roi.width = mask.cols/2;
	roi.height = mask.rows/2; 
	mask.copyTo(frm);
	findContours(frm, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int indx = findBiggestContour(contours);
	vector<vector<Point> >hull(contours.size());
	double x=0 ; double y=0;
	if(indx != -1 )
	{
		convexHull( Mat(contours[indx]), hull[indx], false );
		
		for(int i=0; i<contours[indx].size();i++)
		{
			x += contours[indx][i].x;
			y += contours[indx][i].y;
		}
	
		x /= contours[indx].size();
		y /= contours[indx].size();

		//printf("x= "); cout << x; printf(" y= "); cout << y;printf("\n");
	}
	moveCursor(x ,y);
	rectangle(bgr, Rect((int)x-30, (int)y-30, 60, 60 ), cvScalar(0, 255, 0, 0));

	drawContours(bgr, hull, indx, Scalar(0,250,0), 2 , 8, vector<Vec4i>(), 0, Point());
	
	if(!notInitControlPanel || !notInitScanModePanel)
	{
		imshow(DEPTH_NAME_WINDOW, mask);
	}
	imshow("bgr", bgr);
}

void detectColorObject(Mat& bgrImage, Mat& depthMap, Rect& excludeROI)
{
	inRange(bgrImage, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), depthMap); //Color Filter
	//printf("iLowH= "); cout << iLowH; printf(" iLowS= "); cout << iLowS; printf("iLowV= "); cout << iLowV; printf("\n");
	//printf("iHighH= "); cout << iHighH; printf(" iHighS= "); cout << iHighS; printf("iHighV= "); cout << iHighV; printf("\n");
	depthMap(excludeROI) = Scalar (0);
	//imshow("InDetectColorOnbjectWithoutFace",depthMap);
	detectObj(depthMap, bgrImage.clone());
}

void destroyScanModeWindow()
{
	destroyWindow(SCAN_NAME_WINDOW);
	notInitScanModePanel = true;
	if(notInitControlPanel)
	{
		destroyWindow(DEPTH_NAME_WINDOW);
	}
}

void scanMode()
{
	namedWindow(SCAN_NAME_WINDOW, 1);
	Rect roiScan(0,0,bgrImage.cols/2, bgrImage.rows/2) ;
	initHandColor(bgrImage(roiScan));
	notInitScanModePanel = false;
}

void initControlPanel()
{
	if(notInitControlPanel)
	{
		trackBarSettings["LowH"] = &iLowH;
		trackBarSettings["HighH"] = &iHighH;
		trackBarSettings["LowS"] = &iLowS;
		trackBarSettings["HighS"] = &iHighS;
		trackBarSettings["LowV"] = &iLowV;
		trackBarSettings["HighV"] = &iHighV;
		notInitControlPanel = false;
	}
}

void destroyControlPanelWindow()
{
	if(!notInitControlPanel )
	{
		destroyWindow(CONTROL_NAME_WINDOW);
		notInitControlPanel = true;
		if(notInitScanModePanel)
		{
			destroyWindow(DEPTH_NAME_WINDOW);
		}
	}
}

void colorControlPanel()
{
	if(notInitControlPanel)
	{
		initControlPanel();
		namedWindow(CONTROL_NAME_WINDOW, CV_WINDOW_AUTOSIZE); //create a window called "Control"
	
		for(auto  iterator = trackBarSettings.begin(); iterator != trackBarSettings.end(); iterator++) 
		{
			cvCreateTrackbar((*iterator).first, CONTROL_NAME_WINDOW, (*iterator).second, 255); 
		}
	}
	else
	{
		destroyControlPanelWindow();
	}
}

