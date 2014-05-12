//TODO: consider IFNDEFs
#include "opencv2/opencv.hpp"
#include "Classifier.h"

using namespace cv;
using namespace std;

int main(int, char**)
{
	//VideoCapture colorStream("D:\\Torrents\\Hotel Rwanda\\HotelRwanda.avi"); // open the default camera
	//VideoCapture colorStream("D:/Color/%d_Color.bmp");
	VideoCapture colorStream("C:/KinectData/Color/%d_Color.bmp"); 
	if (!colorStream.isOpened())  // check if we succeeded
		return -1;

	//VideoCapture depthStream("D:/Depth/%d_depth.bmp");
	VideoCapture depthStream("C:/KinectData/Depth/%d_depth.bmp");
	if (!depthStream.isOpened())  // check if we succeeded
		return -2;
	

	Mat edges;
	namedWindow("edges", 1);
	namedWindow("depth", 1);

	int prevClass = 0;
	int curClass = 0;

	for (;;)
	{
		Mat colorFrame;
		Mat depthFrame;
		colorStream >> colorFrame; // get a new frame from camera
		depthStream >> depthFrame; // get a new frame from camera
		if (colorFrame.empty())
		{
			waitKey(5000);
			break;
		}
		if (depthFrame.empty())
		{
			waitKey(5000);
			break;
		}

		// can process the streams here however i want
		curClass = classify(colorFrame, depthFrame);
		if (curClass != prevClass) {
			printClass(curClass);
			prevClass = curClass;
		}

		cvtColor(colorFrame, edges, CV_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);
		imshow("edges", edges);
		imshow("depth", depthFrame);
		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}
