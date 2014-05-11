#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMENSIONS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6


using namespace cv;
using namespace std;

float readfloat(FILE *f) {
	float v;
	fread((void*)(&v), sizeof(v), 1, f);
	return v;
}

int main(int, char**) {
	ifstream ifs;

	// Open the binary file that contains joint data
	ifs.open("C:\\KinectData\\Skel\\Joint_Position.binary", ios::in | ios::binary);
	if (ifs.is_open()) {
		cout << "yay" << endl;
	}
	
	// 3d array to store all joint data
	float positions[NUM_SKEL_TRACKED][NUM_SKEL_JOINTS][NUM_JOINT_DIMENSIONS];
	float position = 1;

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
		int frameLength = sizeof(float)* NUM_SKEL_TRACKED * NUM_SKEL_JOINTS * NUM_JOINT_DIMENSIONS;
		cout << "frame length: " << frameLength << " characters... " << endl;

		while (!ifs.eof()) {
			
			// read data as a block:
			//ifs.read(buffer, length);
			// Read in one frame of skeleton data (includes 6 skeletons)
			ifs.read((char *)positions, frameLength);

			if (ifs)
				std::cout << "all characters read successfully.";
			else
				std::cout << "error: only " << ifs.gcount() << " could be read";
			

			// ...buffer contains the entire file...
			//joints = Mat(3, sizes, CV_32F, (float *)buffer);

			// Loop to print out contents of a 3d array
			/*int count = 0;
			for (int i = 0; i < NUM_SKEL_TRACKED; ++i) {
				cout << "skel " << i << ": " << endl;
				for (int j = 0; j < NUM_SKEL_JOINTS; ++j) {
					cout << "x,y,z,w " << j << ": " << endl;
					for (int k = 0; k < NUM_JOINT_DIMENSIONS; ++k) {
						cout << positions[i][j][k] << ", ";
						//cout << joints.at<float>(i, j, k) << ", ";
						//cout << ((float *)buffer)[count] << ", ";
						//++count;
					}
				}
				cout << endl << endl;
			}*/

			// 2d openCV Mat to contain joint data
			Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMENSIONS, CV_32F, &positions[2][0][0]);

			// Loop to print out contents of a 2d Mat
			/*for (int j = 0; j < NUM_SKEL_JOINTS; ++j) {
			cout << "x,y,z,w " << j << ": " << endl;
			for (int k = 0; k < NUM_JOINT_DIMENSIONS; ++k) {
			cout << joints.at<float>(j, k) << ", " << endl;
			}
			cout << endl;
			}*/

			// Matrices for display (image)
			int imsize = 300;
			Mat edges = Mat::eye(imsize, imsize, CV_8UC3);

			// Loop to draw joints onto display Mat
			for (int i = 0; i < NUM_SKEL_JOINTS; ++i) {
				//float x = positions[2][i][0];
				//float y = positions[2][i][1];
				float x = joints.at<float>(i, 0);
				float y = joints.at<float>(i, 1);
				x = x * (imsize / 3) + (imsize / 2);
				y = y * -(imsize / 3) + (imsize / 2);
				Point pt = Point((int)x, (int)y);
				circle(edges, pt, 2, Scalar(255, 100, 100), -1);
			}

			imshow("edges", edges);
			waitKey(30);
			//destroyAllWindows();
		}
	}
	ifs.close();

	

	return 0;
}
