//This program is for saving xml and yml files of the accumulated samples. Also for SVM, although that
// is likely to be moved to the ModelTester program, which will also contain feature extractor.

#include "ModelTrainer.h"
#include <opencv2/ml/ml.hpp>
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
	long long timeAccumulator[FRAME_CAP][NUM_TIME_DIMS] = { 0 };
	// Chose a large frame cap suitable for sample frame sizes for this project. If larger number of frames needed, consider using dynamic allocation.

	char exerciseType = 'j';
	char mainDir;
	String exerciseTypeName;
	String mainDirName;
	int numReps;

	cout << "Enter which main directory to use. 'r' for RyanData, 't' for ThesisData: " << endl;
	std::cin >> mainDir;

	if (mainDir == 'r') {
		mainDirName = "RyanData";
	}
	else {
		mainDirName = "ThesisData";
	}

	std::cout << "What type of exercise? 'u' arm curl, 'i' arm circle, 'j' jumping jack" << endl;
	std::cin >> exerciseType;

	if (exerciseType == 'j') {
		exerciseTypeName = "Jumping jack";
		numReps = 33; // TODO: this is ONLY TRUE if main dir is RyanData. will break for ThesisData
	}
	else if (exerciseType == 'u') {
		exerciseTypeName = "Arm curl";
		numReps = 18; // TODO: this is ONLY TRUE if main dir is RyanData. will break for ThesisData
	}
	else {
		exerciseTypeName = "Arm circle";
		numReps = 18; // TODO: this is ONLY TRUE if main dir is RyanData. will break for ThesisData
	}

	int directoryNum = 0;
	int numFilesRead = 0;
	int totalFrameNum = 0; //TODO: can probably update this once after each rep file
	int frameNum; 
	char waitin = 'y';
	bool noMoreFilesInDirectory = false;
	bool directoryEmpty = false;
	vector<int> frameCounts;

	while (waitin == 'y') {
		directoryNum++; //TODO: restructure code so that I increment this and numFilesRead at the ends of the loops
		numFilesRead = 0;

		while (waitin == 'y') {
			numFilesRead++;
			frameNum = 0;

			String fileNamePosition = "D:\\" + mainDirName + "\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Position" + to_string(numFilesRead);
			String fileNameOrientation = "D:\\" + mainDirName + "\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Orientation" + to_string(numFilesRead);
			String fileNameTime = "D:\\" + mainDirName + "\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Time" + to_string(numFilesRead);

			// Open the binary files that contain the data
			positionInputFileStream.open(fileNamePosition, ios::in | ios::binary);
			orientationInputFileStream.open(fileNameOrientation, ios::in | ios::binary);
			timeInputFileStream.open(fileNameTime, ios::in | ios::binary);
			if (timeInputFileStream.is_open() && positionInputFileStream.is_open() && orientationInputFileStream.is_open()) {
				std::cout << "All three files opened okay." << endl;
				noMoreFilesInDirectory = false;
			}
			else {
				//cout << "Couldnt open one or more of the files" << endl;
				if (noMoreFilesInDirectory) {
					directoryEmpty = true;
				}
				noMoreFilesInDirectory = true;
				break;
			}

			if (positionInputFileStream && timeInputFileStream && orientationInputFileStream) {
				int jointFrameLength = sizeof(float)* NUM_SKEL_JOINTS * NUM_JOINT_DIMS;
				int timeFrameLength = sizeof(long long)* NUM_TIME_DIMS;
				//totalFrameNum = 0;

				while (!positionInputFileStream.eof() && !timeInputFileStream.eof() && !orientationInputFileStream.eof()) { //TODO: figure out why this goes 1 iteration too long
					// Read in one frame of skeleton data (includes 6 skeletons)
					positionInputFileStream.read((char *)positions, jointFrameLength);
					orientationInputFileStream.read((char *)orientations, jointFrameLength);
					timeInputFileStream.read((char *)times, timeFrameLength);

					if (!positionInputFileStream || !timeInputFileStream || !orientationInputFileStream) {
						//cout << "error: couldnt read data from (probably end of) file." << endl;
					}
					else {
						// Copy joint data into the accumulator buffer
						if (totalFrameNum < FRAME_CAP) { //TODO: fix hardcoding
							memcpy(&positionAccumulator[totalFrameNum], positions, jointFrameLength);
							memcpy(&orientationAccumulator[totalFrameNum], orientations, jointFrameLength);
							memcpy(&timeAccumulator[totalFrameNum], times, timeFrameLength);
						}
						else {
							std::cout << "Warning: exceeded frame cap for accumulator" << endl;
						}
						totalFrameNum++;
						frameNum++;
					}

					//displayJoints((float *)positions);
				}
				frameCounts.push_back(frameNum);

				positionInputFileStream.close();
				orientationInputFileStream.close();
				timeInputFileStream.close();
			}
		}

		if (directoryEmpty || waitin != 'y') {
			std::cout << endl << "frames read: " << totalFrameNum << endl;
			std::cout << "All directories and files read. Press any key to finish." << endl;
			cin >> waitin;
			break;
		}
	}
	// At this point the accumulators have all the frames in them.
	// Now we read in the labels per exercise repetition (rep).

	ifstream labelFile;
	labelFile.open("D:\\" + mainDirName + "\\" + exerciseTypeName + "\\labels.txt", ios::in); // TODO: only works for RyanData i think
	float labelsFromFile[MAX_REPS]; // TODO: fix hardcoding

	for (int i = 0; i < numReps; ++i) { // TODO: fix hardcoding
		labelFile >> labelsFromFile[i];
	}

	float labelsArr[FRAME_CAP];

	int k = 0;
	for (int i = 0; i < frameCounts.size(); ++i) { //TODO: should check if frameCounts.size is equal to labelsfromfile length
		for (int j = 0; j < frameCounts[i]; ++j) {
			labelsArr[k] = labelsFromFile[i];
			++k;
		}
	}

	Mat labels = Mat(totalFrameNum, 1, CV_32F, labelsArr);
	
	//All joints
	Mat pos_samples = Mat(totalFrameNum, NUM_SKEL_JOINTS * NUM_JOINT_DIMS, CV_32F, positionAccumulator);
	Mat ori_samples = Mat(totalFrameNum, NUM_SKEL_JOINTS * NUM_JOINT_DIMS, CV_32F, orientationAccumulator);

	// Commented out for safety reasons. But the code below is pretty much the point of this program.
	/*__int32 truncTimes[FRAME_CAP][NUM_TIME_DIMS];
	for (int i = 0; i < totalFrameNum; ++i) {
		for (int j = 0; j < NUM_TIME_DIMS; ++j) {
			truncTimes[i][j] = (__int32)timeAccumulator[i][j]; //TODO: be careful with typecasting into smaller data types, might not work on all machines?
		}
	}

	//Mat timesMat = Mat(totalFrameNum, NUM_TIME_DIMS, DataType<long long>::type, timeAccumulator);
	Mat timesMat = Mat(totalFrameNum, NUM_TIME_DIMS, CV_32S, truncTimes);

	cv::FileStorage file("D:\\" + mainDirName + "\\" + exerciseTypeName + "\\time_samples.xml", cv::FileStorage::WRITE);
	file << "times" << timesMat;
	
	cv::FileStorage file2("D:\\" + mainDirName + "\\" + exerciseTypeName + "\\pos_samples.xml", cv::FileStorage::WRITE);
	file2 << "positions" << pos_samples;

	cv::FileStorage file3("D:\\" + mainDirName + "\\" + exerciseTypeName + "\\ori_samples.xml", cv::FileStorage::WRITE);
	file3 << "orientations" << ori_samples;

	cv::FileStorage file4("D:\\" + mainDirName + "\\" + exerciseTypeName + "\\all_labels.xml", cv::FileStorage::WRITE);
	file4 << "labels" << labels;*/

	cv::destroyAllWindows();

	return 0;
}
