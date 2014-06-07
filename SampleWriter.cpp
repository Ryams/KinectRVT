// Program to read in data saved with KinectStreamSaver, and then user goes through
// each frame of data from that binary file and writes out selected exercise repetitions to multiple files
// in that same directory.

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

	String exerciseType;
	String mainDirName;
	int dataDirectoryNum = 1;
	int skelTrackNum = 2;
	char waitin;
	char mainDir;

	cout << "Enter which main directory to use. 'r' for RyanData, 't' for ThesisData: " << endl;
	std::cin >> mainDir;

	if (mainDir == 'r') {
		mainDirName = "RyanData";
	}
	else {
		mainDirName = "ThesisData";
	}

	cout << "Enter the number of the data directory" << endl;
	std::cin >> dataDirectoryNum;

	cout << "Enter 'j' jumping jack, 'i' arm circle, 'u' arm curl" << endl; //TODO: possible error checking on all these inputs
	std::cin >> waitin;

	if (waitin == 'j') {
		exerciseType = "Jumping jack";
	}
	else if (waitin == 'u') {
		exerciseType = "Arm curl";
	}
	else {
		exerciseType = "Arm circle";
	}

	cout << "Enter the number of the skeleton tracking number" << endl;
	std::cin >> skelTrackNum;

	// Open the binary files that contain the data
	ifs_joint_pos.open("D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Skel\\Joint_Position.binary", ios::in | ios::binary);
	cout << "D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Skel\\Joint_Position.binary" << endl;
	ifs_joint_orie.open("D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Skel\\Joint_Orientation.binary", ios::in | ios::binary);
	ifs_time.open("D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Skel\\liTimeStamp.binary", ios::in | ios::binary);
	if (ifs_time.is_open() && ifs_joint_pos.is_open() && ifs_joint_orie.is_open()) {
		cout << "yay x3" << endl;
		cout << "Welcome to the file writer. Press any key to proceed to the next frame." << endl
			<< "Press 's' to save the previous frames, and 'd' to discard them." << endl;
	}
	else {
		cout << "Could not open all the files. Please check that you have entered the correct information." << endl;
		std::cin >> waitin;
		return 0;
	}

	// 3d array to store joint data
	float positions[NUM_SKEL_TRACKED][NUM_SKEL_JOINTS][NUM_JOINT_DIMS];
	float orientations[NUM_SKEL_TRACKED][NUM_SKEL_JOINTS][NUM_JOINT_DIMS];
	// Array to store time data
	long long times[NUM_TIME_DIMS] = { 0 };
	// Array to store cumulative data over multiple frames
	float allPositions[170][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	float allOrientations[170][NUM_SKEL_JOINTS * NUM_JOINT_DIMS] = { 0 }; //TODO: fix hardcoding.
	long long allTimes[170][NUM_TIME_DIMS] = { 0 };
	// Chose a large number suitable for sample frame sizes for this project. If larger number of frames needed, consider using dynamic allocation.

	String fileOutBaseNamePosition = "D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Position";
	String fileOutBaseNameOrientation = "D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Orientation";
	String fileOutBaseNameTime = "D:\\" + mainDirName + "\\Data" + to_string(dataDirectoryNum) + "\\" + exerciseType + "\\Time";

	int numFilesWritten = 0;

	if (ifs_joint_pos && ifs_time && ifs_joint_orie) {
		int jointFrameLength = sizeof(float)* NUM_SKEL_TRACKED * NUM_SKEL_JOINTS * NUM_JOINT_DIMS;
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
					memcpy(&allPositions[frameNum], &positions[skelTrackNum][0][0], sizeof(float)* NUM_SKEL_JOINTS * NUM_JOINT_DIMS);
					memcpy(&allOrientations[frameNum], &orientations[skelTrackNum][0][0], sizeof(float)* NUM_SKEL_JOINTS * NUM_JOINT_DIMS);
					memcpy(&allTimes[frameNum], times, timeFrameLength);
				}
				frameNum++;
			}

			// 2d openCV Mat to contain joint data
			Mat joints = Mat(NUM_SKEL_JOINTS, NUM_JOINT_DIMS, CV_32F, &positions[skelTrackNum][0][0]);
			

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

			if (frameNum < 100) {
				cout << "        times: " << endl;
				for (int i = 0; i < 5; ++i) {
					cout << allTimes[0][i] << " ";
				}
				cout << endl;
			}

			char inChar = waitKey(0);

			if (inChar == 's') {
				++numFilesWritten;
				cout << "Writing file " << numFilesWritten << ": " << endl << getFileName(fileOutBaseNamePosition, numFilesWritten) << endl
					<< getFileName(fileOutBaseNameOrientation, numFilesWritten) << endl
					<< getFileName(fileOutBaseNameTime, numFilesWritten) << endl;

				ofs_out_posi.open(getFileName(fileOutBaseNamePosition, numFilesWritten), ios_base::out | ios::binary);
				ofs_out_posi.write((char *)allPositions, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum); //double check frameNum is right

				ofs_out_orie.open(getFileName(fileOutBaseNameOrientation, numFilesWritten), ios_base::out | ios::binary);
				ofs_out_orie.write((char *)allOrientations, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum);

				ofs_out_time.open(getFileName(fileOutBaseNameTime, numFilesWritten), ios_base::out | ios::binary);
				ofs_out_time.write((char *)allTimes, timeFrameLength * frameNum);

				//Do some kind of mem-clear on allPoisitions, allOrientations, allTimes
				memset((char *)allPositions, 0, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum);
				memset((char *)allOrientations, 0, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum);
				memset((char *)allTimes, 0, sizeof(float) * NUM_TIME_DIMS * frameNum); // TODO: this is wrong, should be timeFrameLength or sizeof(long long) * frameNum
				frameNum = 0;

				ofs_out_posi.close();
				ofs_out_orie.close();
				ofs_out_time.close();
			}
			else if (inChar == 'd') { // Discard this set of frames, does not write to file
				//Do some kind of mem-clear on allPoisitions, allOrientations, allTimes
				cout << "Discarded frames" << endl;
				memset((char *)allPositions, 0, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum);
				memset((char *)allOrientations, 0, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum);
				memset((char *)allTimes, 0, sizeof(float) * NUM_TIME_DIMS * frameNum); // TODO: this is wrong, should be timeFrameLength or sizeof(long long) * frameNum
				frameNum = 0;
			}
		}
		cout << endl << "would you like to save the last set of frames? Press 's' if yes." << endl;
		char waitin;
		std::cin >> waitin;
		if (waitin == 's') {
			++numFilesWritten;
			cout << "Writing file " << numFilesWritten << ": " << endl << getFileName(fileOutBaseNamePosition, numFilesWritten) << endl
				<< getFileName(fileOutBaseNameOrientation, numFilesWritten) << endl
				<< getFileName(fileOutBaseNameTime, numFilesWritten) << endl;

			ofs_out_posi.open(getFileName(fileOutBaseNamePosition, numFilesWritten), ios_base::out | ios::binary);
			ofs_out_posi.write((char *)allPositions, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum); //double check frameNum is right

			ofs_out_orie.open(getFileName(fileOutBaseNameOrientation, numFilesWritten), ios_base::out | ios::binary);
			ofs_out_orie.write((char *)allOrientations, sizeof(float) * NUM_SKEL_JOINTS * NUM_JOINT_DIMS * frameNum);

			ofs_out_time.open(getFileName(fileOutBaseNameTime, numFilesWritten), ios_base::out | ios::binary);
			ofs_out_time.write((char *)allTimes, timeFrameLength * frameNum);

			ofs_out_posi.close();
			ofs_out_orie.close();
			ofs_out_time.close();
		}
		//std::cin >> waitin;
	}

	ifs_joint_pos.close();
	ifs_joint_orie.close();
	ifs_time.close();
	//destroyAllWindows();

	return 0;
}
