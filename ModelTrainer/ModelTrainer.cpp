#include "ModelTrainer.h"
#include <iostream>
#include <fstream> 

using namespace cv;
using namespace std;


//TODO: Move most of the file data reading code to a new class/ file
int main(int, char**) {
	ifstream positionInputFileStream, orientationInputFileStream, timeInputFileStream;

	// 3d arrays to store data
	float positions[NUM_SKEL_JOINTS][NUM_JOINT_DIMS] = { 0 };
	float orientations[NUM_SKEL_JOINTS][NUM_JOINT_DIMS] = { 0 };
	long long times[5] = { 0 };
	// Array to store cumulative data over multiple frames
	float positionAccumulator[FRAME_CAP][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float orientationAccumulator[FRAME_CAP][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float timeAccumulator[FRAME_CAP][NUM_TIME_DIMS] = { 0 };
	// Chose a large frame cap suitable for sample frame sizes for this project. If larger number of frames needed, consider using dynamic allocation.

	char exerciseType = 'j';
	cout << "What type of exercise? 'u' arm curl, 'i' arm circle, 'j' jumping jack" << endl;
	std::cin >> exerciseType;

	String exerciseTypeName;

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
	int numFilesRead = 0;
	int frameNum = 0;
	char waitin = 'y';
	bool noMoreFilesInDirectory = false;
	bool directoryEmpty = false;

	while (waitin == 'y') {
		directoryNum++; //TODO: restructure code so that I increment this and numFilesRead at the ends of the loops
		numFilesRead = 0;

		while (waitin == 'y') {
			numFilesRead++;

			String fileNamePosition = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Position" + to_string(numFilesRead);
			String fileNameOrientation = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Orientation" + to_string(numFilesRead);
			String fileNameTime = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Time" + to_string(numFilesRead);

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
				//frameNum = 0;

				while (!positionInputFileStream.eof() && !timeInputFileStream.eof() && !orientationInputFileStream.eof()) { //TODO: figure out why this goes 1 iteration too long
					// Read in one frame of skeleton data (includes 6 skeletons)
					positionInputFileStream.read((char *)positions, jointFrameLength);
					orientationInputFileStream.read((char *)orientations, jointFrameLength);
					timeInputFileStream.read((char *)times, timeFrameLength);

					if (!positionInputFileStream || !timeInputFileStream || !orientationInputFileStream) {
						cout << "error: couldnt read data from (probably end of) file." << endl;
					}
					else {
						// Copy joint data into the accumulator buffer
						if (frameNum < FRAME_CAP) { //TODO: fix hardcoding
							memcpy(&positionAccumulator[frameNum], positions, jointFrameLength);
							memcpy(&orientationAccumulator[frameNum], orientations, jointFrameLength);
							memcpy(&timeAccumulator[frameNum], times, timeFrameLength);
						}
						frameNum++;
					}

					displayJoints((float *)positions);
				}

				positionInputFileStream.close();
				orientationInputFileStream.close();
				timeInputFileStream.close();
			}
		}

		if (directoryEmpty || waitin != 'y') {
			cout << "All directories and files read. Press any key to finish." << endl;
			cin >> waitin;
			break;
		}
	}
	destroyAllWindows();

	return 0;
}
