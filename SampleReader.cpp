#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5


using namespace cv;
using namespace std;


String getFileName(const string &fileName, const int &fileNum) {
	String fileout = fileName;
	fileout += to_string(fileNum);
	return fileout;
}

//TODO: Move most of the file data reading code to a new class/ file
int main(int, char**) {
	ifstream ifs_joint_pos, ifs_joint_orie, ifs_time; //TODO: rename these
	ofstream ofs_out_posi, ofs_out_orie, ofs_out_time;

	// Open the binary files that contain the data
	ifs_joint_pos.open("D:\\ThesisData\\Data1\\Arm circle\\Position1", ios::in | ios::binary);
	ifs_joint_orie.open("D:\\ThesisData\\Data1\\Arm circle\\Orientation1", ios::in | ios::binary);
	ifs_time.open("D:\\ThesisData\\Data1\\Arm circle\\Time1", ios::in | ios::binary);
	if (ifs_time.is_open() && ifs_joint_pos.is_open() && ifs_joint_orie.is_open()) {
		cout << "yay x3" << endl;
	}

	// 3d array to store joint data
	float positions[NUM_SKEL_JOINTS][NUM_JOINT_DIMS];
	float orientations[NUM_SKEL_JOINTS][NUM_JOINT_DIMS];
	// Array to store time data
	long long times[5] = { 0 };
	// Array to store cumulative data over multiple frames
	float allPositions[170][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float allOrientations[170][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float allTimes[170][NUM_TIME_DIMS] = { 0 };
	// Chose a large number suitable for sample frame sizes for this project. If larger number of frames needed, consider using dynamic allocation.

	String fileBaseNamePosition = "D:\\ThesisData\\Data1\\Arm circle\\Position";
	String fileBaseNameOrientation = "D:\\ThesisData\\Data1\\Arm circle\\Orientation";
	String fileBaseNameTime = "D:\\ThesisData\\Data1\\Arm circle\\Time";

	int numFilesRead = 0;

	if (ifs_joint_pos && ifs_time && ifs_joint_orie) {
		int jointFrameLength = sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS;
		int timeFrameLength = sizeof(long long) * NUM_TIME_DIMS;
		int frameNum = 0;

		while (!ifs_joint_pos.eof() && !ifs_time.eof() && !ifs_joint_orie.eof()) { //TODO: figure out why this goes 1 iteration too long
			// Read in one frame of skeleton data (includes 6 skeletons)
			ifs_joint_pos.read((char *)positions, jointFrameLength);
			ifs_joint_orie.read((char *)orientations, jointFrameLength);
			ifs_time.read((char *)times, timeFrameLength);

			if (!ifs_joint_pos || !ifs_time || !ifs_joint_orie) {
				cout << "error: only " << ifs_joint_pos.gcount() << " could be read from pos" << endl;
				cout << "error: only " << ifs_joint_orie.gcount() << " could be read from orie" << endl;
				cout << "error: only " << ifs_time.gcount() << " could be read from time" << endl;
			}
			else {
				// Copy joint data into the accumulator buffer
				if (frameNum < 169) { //TODO: fix hardcoding
					memcpy(&allPositions[frameNum], positions, jointFrameLength);
					memcpy(&allOrientations[frameNum], orientations, jointFrameLength);
					memcpy(&allTimes[frameNum], times, timeFrameLength);
				}
				frameNum++;
			}

			if (frameNum < 3) {
				cout << "        orientations: " << endl;
				for (int i = 0; i < NUM_SKEL_JOINTS; ++i) {
					for (int j = 0; j < NUM_JOINT_DIMS; ++j) {
						cout << orientations[i][j] << " ";
					}
					cout << endl;
				}
				cout << "        times: " << endl;
				for (int i = 0; i < 5; ++i) {
					cout << times[i] << " ";
				}
				cout << endl;
			}

			// 2d openCV Mat to contain joint data
			Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMS, CV_32F, positions);


			// Matrices for display (image)
			int imsize = 300;
			Mat edges = Mat::eye(imsize, imsize, CV_8UC3); //TODO: doesnt need to be eye

			// Loop to draw joints onto display Mat
			for (int i = 0; i < NUM_SKEL_JOINTS; ++i) {
				float x = joints.at<float>(i, 0);
				float y = joints.at<float>(i, 1);
				// Scale x, y to fit nicely in display window
				x = x * (imsize / 3) + (imsize / 2);
				y = y * -(imsize / 3) + (imsize / 2);
				Point pt = Point((int)x, (int)y);
				circle(edges, pt, 2, Scalar(255, 100, 100), -1);
			}

			imshow("edges", edges);
			char inChar = waitKey(30); // 30 frames per second?
		}

		char waitin;
		cin >> waitin;
	}

	ifs_joint_pos.close();
	ifs_joint_orie.close();
	ifs_time.close();
	//destroyAllWindows();

	return 0;
}
