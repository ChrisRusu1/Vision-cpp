#include <sstream>
#include <conio.h>
#include <string>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <cmath>
using namespace std;

using namespace cv;
//initial min and max HSV filter values.
int H_MIN = 0;
int H_MAX = 30;
int S_MIN = 147;
int S_MAX = 256;
int V_MIN = 124;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;
//names that will appear at the top of each window


const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed





}
string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}

void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &imagebox) {//cam feed

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	findContours(temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));



	RNG rng(12345);
	vector<RotatedRect> minRect(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
	}
	
	//Mat drawing = Mat::zeros(threshold.size(), CV_8UC3);
	
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;
				

			}
			if (objectFound == true) {
				putText(imagebox, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				for (int i = 0; i < contours.size(); i++)
				{
					//imshow("box", drawing);
					Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
					// rotated rectangle
					Point2f rect_points[4]; minRect[i].points(rect_points);
					for (int j = 1; j < 2; j++) {
						line(imagebox, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
					}
					circle(imagebox, rect_points[1], 20, Scalar(0, 255, 0), 1, 8, 0);
					double h1 = rect_points[1].y;
					double h2 = rect_points[2].y;
					double hd = h2 - h1;
					double v1 = rect_points[1].x;
					double v2 = rect_points[2].x;
					double vd = v2 - v1;
					double hd2 = pow(hd, 2);
					double vd2 = pow(vd, 2);
					double hyp2 = hd2 + vd2;
					double hyp = sqrt(hyp2);
					double sinl = vd / hyp;
					double sinrad = asin(sinl);
					double sin = sinrad * 180 / 3.14;
					double finala = 90 - sin;
					cout << finala << " degrees (angle from horizontal to line)" << endl;

				}
				//w = 640
				//h = 480
			}

		}

		else putText(imagebox, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
int main(int argc, char* argv[])
{
	bool trackObjects = true;
	bool useMorphOps = true;
	Mat cameraFeed;
	Mat HSV;
	Mat threshold;
	Mat imagebox = imread("C:\\Users\\ZeePu\\source\\repos\\object tracking test\\object tracking test\\Box.jpg", CV_LOAD_IMAGE_COLOR);

	int x = 0, y = 0;
	createTrackbars();
	VideoCapture capture;
	capture.open(0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	while (1) {
		capture.read(cameraFeed);
		cvtColor(imagebox, HSV, COLOR_BGR2HSV);//camfeed
		//threshold matrix
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		if (useMorphOps)
			morphOps(threshold);
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);
		
		imshow("box", imagebox);
	
		imshow(windowName2, threshold);
		
		//imshow(windowName, cameraFeed);
		//imshow(windowName1, HSV);
		

		
		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
	}






	return 0;
}
