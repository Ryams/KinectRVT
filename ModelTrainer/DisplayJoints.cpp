#include "ModelTrainer.h"

using namespace cv;
using namespace std;

void displayJoints(float *positions) { // Bothers me that positions cant be const
	// 2d openCV Mat to contain joint data
	Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMS, CV_32F, positions);
	// Matrices for display (image)
	Mat edges = Mat::eye(IMZISE, IMZISE, CV_8UC3); //TODO: doesnt need to be eye

	// Loop to draw joints onto display Mat
	for (int i = 0; i < NUM_SKEL_JOINTS; ++i) {
		float x = joints.at<float>(i, 0);
		float y = joints.at<float>(i, 1);
		// Scale x, y to fit nicely in display window
		x = x * (IMZISE / 3) + (IMZISE / 2);
		y = y * -(IMZISE / 3) + (IMZISE / 2);
		Point pt = Point((int)x, (int)y);
		circle(edges, pt, 2, Scalar(255, 100, 100), -1);
	}

	imshow("edges", edges);
	char inChar = waitKey(30); // 30 frames per second?
}