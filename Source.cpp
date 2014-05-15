#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMENSIONS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6


using namespace cv;
using namespace std;

int main(int, char**) {
	ifstream ifs;
	ifstream ifs2; //TODO: rename these

	// Open the binary file that contains joint data
	ifs.open("C:\\KinectData\\Skel\\Joint_Position.binary", ios::in | ios::binary);
	if (ifs.is_open()) {
		cout << "yay" << endl;
	}
	
	// Open the file that contains timing data
	ifs2.open("C:\\KinectData\\Skel\\liTimeStamp.binary", ios::in | ios::binary);
	if (ifs2.is_open()) {
		cout << "yay2" << endl;
	}

	// 3d array to store all joint data
	float positions[NUM_SKEL_TRACKED][NUM_SKEL_JOINTS][NUM_JOINT_DIMENSIONS];
	// Array to store time data
	long long times[5] = { 0 };
	

	// 2d openCV Mat to contain joint data
	//Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMENSIONS, CV_32F, &positions[2][0][0]);

	// 3d openCV Mat to contain joint data
	//int sizes[] = { NUM_SKEL_TRACKED, NUM_SKEL_JOINTS, NUM_JOINT_DIMENSIONS};
	//Mat joints;

	if (ifs) {
		// get length of file:
		ifs.seekg(0, ifs.end);
		int length = ifs.tellg();
		ifs.seekg(0, ifs.beg);
		std::cout << "Total length: " << length << " characters... " << endl;
		int jointFrameLength = sizeof(float)* NUM_SKEL_TRACKED * NUM_SKEL_JOINTS * NUM_JOINT_DIMENSIONS;
		int timeFrameLength = sizeof(long long)* 5;

		while (!ifs.eof() && !ifs2.eof()) { //TODO: figure out why this goes 1 iteration too long
			// Read in one frame of skeleton data (includes 6 skeletons)
			ifs.read((char *)positions, jointFrameLength);
			ifs2.read((char *)times, timeFrameLength);

			if (!ifs) {
				std::cout << "error: only " << ifs.gcount() << " could be read";
			}

			if (!ifs2) {
				std::cout << "error: only " << ifs2.gcount() << " could be read";
			}

			for (int i = 0; i < 5; ++i) {
				cout << times[i] << ", ";
			}

			cout << endl;

			// 2d openCV Mat to contain joint data
			Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMENSIONS, CV_32F, &positions[2][0][0]);

			// Matrices for display (image)
			int imsize = 300;
			Mat edges = Mat::eye(imsize, imsize, CV_8UC3); //TODO: doesnt need to be eye

			// Loop to draw joints onto display Mat
			for (int i = 0; i < NUM_SKEL_JOINTS; ++i) {
				float x = joints.at<float>(i, 0);
				float y = joints.at<float>(i, 1);
				x = x * (imsize / 3) + (imsize / 2);
				y = y * -(imsize / 3) + (imsize / 2);
				Point pt = Point((int)x, (int)y);
				circle(edges, pt, 2, Scalar(255, 100, 100), -1);
			}

			imshow("edges", edges);
			waitKey(30); // 30 frames per second?
		}
	}

	ifs.close();
	ifs2.close();
	//destroyAllWindows();

	return 0;
}
