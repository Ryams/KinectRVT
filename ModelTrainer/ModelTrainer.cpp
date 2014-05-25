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
	std::cout << "What type of exercise? 'u' arm curl, 'i' arm circle, 'j' jumping jack" << endl;
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

			String fileNamePosition = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Position" + to_string(numFilesRead);
			String fileNameOrientation = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Orientation" + to_string(numFilesRead);
			String fileNameTime = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Time" + to_string(numFilesRead);

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
	labelFile.open("D:\\ThesisData\\Jumping jack CSVs\\labels.txt", ios::in);
	float labelsFromFile[30]; // TODO: fix hardcoding

	for (int i = 0; i < 30; ++i) {
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
	Mat samples = Mat(totalFrameNum, NUM_SKEL_JOINTS * NUM_JOINT_DIMS, CV_32F, orientationAccumulator); //TODO: have streamline for whichfeatures using

	for (int i = 0; i < totalFrameNum; ++i) {
		//cout << labels.at<float>(i) << ", ";
		printf("%2.0f, ", labels.at<float>(i));
	}
	std::cout << endl;

	// Set up SVM's parameters
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

	CvSVM SVM;
	SVM.train(samples, labels, Mat(), Mat(), params);
	std::cout << endl << endl << "Prediction results: " << endl << endl;

	float prevClass = 0;
	float response = 0;

	for (int i = 0; i < samples.size().height; ++i) {
		response = SVM.predict(samples.row(i));
		printf("%2.0f, ", response);
	}

	cin >> waitin;

	cv::destroyAllWindows();

	return 0;
}
