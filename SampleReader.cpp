// Program for viewing the segmented exercise repetitions, all at once.
// Can only view one exercise type at a time.
// No writing to files is done here.

#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5
#define FRAME_CAP 200

using namespace cv;
using namespace std;


//TODO: Move most of the file data reading code to a new class/ file
int main(int, char**) {
	ifstream positionInputFileStream, orientationInputFileStream, timeInputFileStream;

	// 3d array to store joint data
	float positions[NUM_SKEL_JOINTS][NUM_JOINT_DIMS];
	float orientations[NUM_SKEL_JOINTS][NUM_JOINT_DIMS];
	// Array to store time data
	long long times[5] = { 0 };
	// Array to store cumulative data over multiple frames
	float allPositions[FRAME_CAP][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float allOrientations[FRAME_CAP][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float allTimes[FRAME_CAP][NUM_TIME_DIMS] = { 0 };
	// Chose a large number suitable for sample frame sizes for this project. If larger number of frames needed, consider using dynamic allocation.

	char exerciseType = 'j';
	cout << "What type of exercise? 'u' arm curl, 'i' arm circle, 'j' jumping jack" << endl;
	std::cin >> exerciseType;

	String exerciseTypeName;
	String mainDirName;
	char mainDir;

	cout << "Enter which main directory to use. 'r' for RyanData, 't' for ThesisData: " << endl;
	std::cin >> mainDir;

	if (mainDir == 'r') {
		mainDirName = "RyanData";
	}
	else {
		mainDirName = "ThesisData";
	}

	if (exerciseType == 'j') {
		exerciseTypeName = "Jumping jack";
	}
	else if (exerciseType == 'u') {
		exerciseTypeName = "Arm curl";
	}
	else {
		exerciseTypeName = "Arm circle";
	}

	int directoryNum = 0;
	int numFilesRead = 0; // TODO: Needs to be reset every time we change directories.
	char waitin = 'y';
	bool noMoreFilesInDirectory = false;
	bool directoryEmpty = false;

	while (waitin == 'y') {
		directoryNum++; //TODO: restructure code so that I increment this and numFilesRead at the ends of the loops
		numFilesRead = 0;

		while (waitin == 'y') {
			numFilesRead++;

			String fileNamePosition = "D:\\" + mainDirName + "\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Position" + to_string(numFilesRead);
			String fileNameOrientation = "D:\\" + mainDirName + "\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Orientation" + to_string(numFilesRead);
			String fileNameTime = "D:\\" + mainDirName + "\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Time" + to_string(numFilesRead);

			// Open the binary files that contain the data
			positionInputFileStream.open(fileNamePosition, ios::in | ios::binary);
			orientationInputFileStream.open(fileNameOrientation, ios::in | ios::binary);
			timeInputFileStream.open(fileNameTime, ios::in | ios::binary);
			if (timeInputFileStream.is_open() && positionInputFileStream.is_open() && orientationInputFileStream.is_open()) {
				cout << "All three files opened okay." << endl;
				noMoreFilesInDirectory = false;
			}
			else {
				cout << "Couldnt open one or more of the files" << endl;
				if (noMoreFilesInDirectory) {
					directoryEmpty = true;
				}
				noMoreFilesInDirectory = true;
				break;
			}

			if (positionInputFileStream && timeInputFileStream && orientationInputFileStream) {
				int jointFrameLength = sizeof(float)* NUM_SKEL_JOINTS * NUM_JOINT_DIMS;
				int timeFrameLength = sizeof(long long)* NUM_TIME_DIMS;
				int frameNum = 0;

				while (!positionInputFileStream.eof() && !timeInputFileStream.eof() && !orientationInputFileStream.eof()) { //TODO: figure out why this goes 1 iteration too long
					// Read in one frame of skeleton data (includes 6 skeletons)
					positionInputFileStream.read((char *)positions, jointFrameLength);
					orientationInputFileStream.read((char *)orientations, jointFrameLength);
					timeInputFileStream.read((char *)times, timeFrameLength);

					if (!positionInputFileStream || !timeInputFileStream || !orientationInputFileStream) {
						cout << "error: only " << positionInputFileStream.gcount() << " could be read from pos" << endl;
						cout << "error: only " << orientationInputFileStream.gcount() << " could be read from orie" << endl;
						cout << "error: only " << timeInputFileStream.gcount() << " could be read from time" << endl;
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
					char inChar = waitKey(30);
				}

				positionInputFileStream.close();
				orientationInputFileStream.close();
				timeInputFileStream.close();
			}

			cout << "Go to next file? ";
			std::cin >> waitin;
		}

		if (directoryEmpty || waitin != 'y') {
			cout << "All directories and files read. Press any key to finish." << endl;
			cin >> waitin;
			break;
		}
		cout << "Go to next data directory?";
		std::cin >> waitin;
	}
	//destroyAllWindows();

	return 0;
}
