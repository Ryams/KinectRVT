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

	//TODO: ask the user at the start if they are looking at arm circle, arm curl, or jumping jacks,
	// and construct these base file names depending on that. Also there should be a loop around this and
	// everything that follows updating the number of data person, ie Data1, Data2, and so on.
	// The file search should proceed beyond the current person's data directory when reading in samples.
	

	int directoryNum = 0;
	int numFilesRead = 0; // TODO: Needs to be reset every time we change directories.
	char waitin = 'y';
	bool noMoreFilesInDirectory = false;
	bool directoryEmpty = false;

	while (waitin == 'y') {
		directoryNum++; //TODO: restructure code so that I increment this and numFilesRead at the ends of the loops

		while (waitin == 'y') {
			numFilesRead++;

			String fileNamePosition = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Position" + to_string(numFilesRead);
			String fileNameOrientation = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Orientation" + to_string(numFilesRead);
			String fileNameTime = "D:\\ThesisData\\Data" + to_string(directoryNum) + "\\" + exerciseTypeName + "\\Time" + to_string(numFilesRead);

			// Open the binary files that contain the data
			ifs_joint_pos.open(fileNamePosition, ios::in | ios::binary);
			ifs_joint_orie.open(fileNameOrientation, ios::in | ios::binary);
			ifs_time.open(fileNameTime, ios::in | ios::binary);
			if (ifs_time.is_open() && ifs_joint_pos.is_open() && ifs_joint_orie.is_open()) {
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

			if (ifs_joint_pos && ifs_time && ifs_joint_orie) {
				int jointFrameLength = sizeof(float)* NUM_SKEL_JOINTS * NUM_JOINT_DIMS;
				int timeFrameLength = sizeof(long long)* NUM_TIME_DIMS;
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

				ifs_joint_pos.close();
				ifs_joint_orie.close();
				ifs_time.close();
			}

			cout << "Go to next file? ";
			std::cin >> waitin;
		}

		if (directoryEmpty) {
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
