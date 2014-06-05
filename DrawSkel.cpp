// Program to view a single skeleton binary file (can be many frames of data).
// File name is hardcoded - so its a single file only.
// Optional basic SVM train/test and output at the bottom.

#include "opencv2/opencv.hpp"
#include <opencv2/ml/ml.hpp>
#include <iostream>
#include <fstream>  
#include "classifier.h"
#define NUM_JOINT_DIMENSIONS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define IMSIZE 300

using namespace cv;
using namespace std;

Point getPointFromIndex(int indx, Mat &joints) {
	float x = joints.at<float>(indx, 0);
	float y = joints.at<float>(indx, 1);
	x = x * (IMSIZE / 3) + (IMSIZE / 2);
	y = y * -(IMSIZE / 3) + (IMSIZE / 2);
	Point pt = Point((int)x, (int)y);

	return pt;
}

// See MSDN's NUI_SKELETON_POSITION_INDEX Enumeration for reference
void drawBones(Mat &edges, Mat &joints) {
	Point pt1 = getPointFromIndex(0, joints);
	Point pt2 = getPointFromIndex(1, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	pt1 = getPointFromIndex(1, joints);
	pt2 = getPointFromIndex(2, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	// Head
	pt1 = getPointFromIndex(2, joints);
	pt2 = getPointFromIndex(3, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	// Left arm
	pt1 = getPointFromIndex(2, joints);
	pt2 = getPointFromIndex(4, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	for (int i = 4; i < 7; ++i) {
		pt1 = getPointFromIndex(i, joints);
		pt2 = getPointFromIndex(i + 1, joints);
		line(edges, pt1, pt2, Scalar(255, 255, 100));
	}

	// Right arm
	pt1 = getPointFromIndex(2, joints);
	pt2 = getPointFromIndex(8, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	for (int i = 8; i < 11; ++i) {
		pt1 = getPointFromIndex(i, joints);
		pt2 = getPointFromIndex(i + 1, joints);
		line(edges, pt1, pt2, Scalar(255, 255, 100));
	}

	// Left leg
	pt1 = getPointFromIndex(0, joints);
	pt2 = getPointFromIndex(12, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	for (int i = 12; i < 15; ++i) {
		pt1 = getPointFromIndex(i, joints);
		pt2 = getPointFromIndex(i + 1, joints);
		line(edges, pt1, pt2, Scalar(255, 255, 100));
	}

	// Right leg
	pt1 = getPointFromIndex(0, joints);
	pt2 = getPointFromIndex(16, joints);
	line(edges, pt1, pt2, Scalar(255, 255, 100));

	for (int i = 16; i < 19; ++i) {
		pt1 = getPointFromIndex(i, joints);
		pt2 = getPointFromIndex(i + 1, joints);
		line(edges, pt1, pt2, Scalar(255, 255, 100));
	}
}

//TODO: Move most of the file data reading code to a new class/ file
int main(int, char**) {
	ifstream ifs_joint;
	ifstream ifs_time; //TODO: rename these

	// Open the binary file that contains joint data
	//ifs_joint.open("C:\\KinectData\\Backup\\Backup2\\Skel\\Joint_Position.binary", ios::in | ios::binary);
	ifs_joint.open("D:\\ThesisData\\Data1\\Arm circle\\Skel\\Joint_Position.binary", ios::in | ios::binary);
	if (ifs_joint.is_open()) {
		cout << "yay" << endl;
	}
	
	// Open the file that contains timing data
	ifs_time.open("C:\\KinectData\\Backup\\Backup2\\Skel\\liTimeStamp.binary", ios::in | ios::binary);
	if (ifs_time.is_open()) {
		cout << "yay2" << endl;
	}

	// 3d array to store all joint data
	float positions[NUM_SKEL_TRACKED][NUM_SKEL_JOINTS][NUM_JOINT_DIMENSIONS];
	// Array to store time data
	long long times[5] = { 0 };
	float allPositions[170][NUM_SKEL_JOINTS * NUM_JOINT_DIMENSIONS] = { 0 };

	if (ifs_joint) {
		// get length of file:
		ifs_joint.seekg(0, ifs_joint.end);
		int length = ifs_joint.tellg();
		ifs_joint.seekg(0, ifs_joint.beg);
		std::cout << "Total length: " << length << " characters... " << endl;
		int jointFrameLength = sizeof(float)* NUM_SKEL_TRACKED * NUM_SKEL_JOINTS * NUM_JOINT_DIMENSIONS;
		int timeFrameLength = sizeof(long long)* 5;

		int k = 0;

		while (!ifs_joint.eof() && !ifs_time.eof()) { //TODO: figure out why this goes 1 iteration too long
			// Read in one frame of skeleton data (includes 6 skeletons)
			ifs_joint.read((char *)positions, jointFrameLength);
			ifs_time.read((char *)times, timeFrameLength);

			if (!ifs_joint) {
				std::cout << "error: only " << ifs_joint.gcount() << " could be read";
			}

			if (!ifs_time) {
				std::cout << "error: only " << ifs_time.gcount() << " could be read";
			}

			// 2d openCV Mat to contain joint data
			Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMENSIONS, CV_32F, &positions[2][0][0]);
			if (k < 169) {
				memcpy(&allPositions[k], &positions[2][0][0], sizeof(float)* NUM_SKEL_JOINTS * NUM_JOINT_DIMENSIONS);
			}
			k++;

			// Matrices for display (image)
			Mat edges = Mat::eye(IMSIZE, IMSIZE, CV_8UC3); //TODO: doesnt need to be eye

			//Draw all the bones.
			drawBones(edges, joints);

			// Loop to draw joints onto display Mat
			for (int i = 0; i < NUM_SKEL_JOINTS; ++i) {
				float x = joints.at<float>(i, 0);
				float y = joints.at<float>(i, 1);
				x = x * (IMSIZE / 3) + (IMSIZE / 2);
				y = y * -(IMSIZE / 3) + (IMSIZE / 2);
				Point pt = Point((int)x, (int)y);
				circle(edges, pt, 2, Scalar(255, 100, 100), -1);
			}

			imshow("edges", edges);
			waitKey(30); // 30 frames per second?
		}
	}

	//All joints
	/*Mat samples = Mat(169, NUM_SKEL_JOINTS * NUM_JOINT_DIMENSIONS, CV_32F, allPositions);
	float labelArr[169]; //TODO: make robust for any size files
	fill_n(labelArr, 169, 1.0);
	for (int i = 85; i < 122; ++i) {
		labelArr[i] = 2.0;
	}
	cout << endl;
	Mat labels = Mat(169, 1, CV_32FC1, labelArr);
	for (int i = 0; i < 169; ++i) {
		//cout << labels.at<float>(i) << ", ";
		printf("%2.0f, ", labels.at<float>(i));
	}
	cout << endl;

	// Set up SVM's parameters
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

	CvSVM SVM;
	SVM.train(samples, labels, Mat(), Mat(), params);

	cout << endl << endl << "Prediction results: " << endl << endl;

	float prevClass = 0;
	float response = 0;

	for (int i = 0; i < samples.size().height; ++i) {
		response = SVM.predict(samples.row(i));
		printf("%2.0f, ", response);
	}*/

	char waitin;
	cin >> waitin;

	ifs_joint.close();
	ifs_time.close();
	//destroyAllWindows();

	return 0;
}
