#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5

using namespace cv;
using namespace std;

// Fill in sample mats with all the accumulated data.
void getSamples(Mat &m1, Mat &m2, Mat &m3) {
	char exerciseType = 'j';
	std::cout << "What type of exercise? 'u' arm curl, 'i' arm circle, 'j' jumping jack" << endl;
	std::cin >> exerciseType;

	String _exerciseTypeName;

	if (exerciseType == 'j') {
		_exerciseTypeName = "Jumping jack";
	}
	else if (exerciseType == 'u') {
		_exerciseTypeName = "Arm curl";
	}
	else {
		_exerciseTypeName = "Arm circle";
	}

	const String exerciseTypeName = String(_exerciseTypeName); //TODO: I had to hack this because there was a weird bug where
	// _exerciseTypeName was getting written over after calling FileStorage fs;

	FileStorage fs("D:\\ThesisData\\" + exerciseTypeName + " CSVs\\time_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m1;
	fs.open("D:\\ThesisData\\" + exerciseTypeName + " CSVs\\pos_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m2;
	fs.open("D:\\ThesisData\\" + exerciseTypeName + " CSVs\\ori_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m3;
	fs.release();
}

// Extract features from raw accumulated sample data. Each row is a data from one frame of video.
// Time has 5 values, posMat has position data. each row is 20 joints, each joint has x, y, z, w where
// w is tracking state of joint. oriMat has orientation data, in w, x, y, z quaternion.
// Here is the most basic features, using the raw orientation data.
void getFeatureVec(Mat &features, const Mat &timeMat, const Mat &posMat, const Mat &oriMat) {
	oriMat.copyTo(features);
}

void getTrainTestSets(Mat &train, Mat &test, Mat &features) {
	

	Mat indices(features.size().height, 1, CV_32S);
	cout << "height: " << features.size().height << "width: " << features.size().width << endl;
	cout << "height: " << indices.size().height << "width: " << indices.size().width << endl;
	for (int i = 0; i < indices.size().height; ++i) {
		indices.at<int>(i) = i;
		cout << indices.at<int>(i) << " ";
	}

	cout << endl << endl;
	randShuffle(indices, 3);
	for (int i = 0; i < indices.size().height; ++i) {
		cout << indices.at<int>(i) << " ";
	}
	cout << endl;
}

int main(int, char**) {
	Mat timeMat, posMat, oriMat;

	getSamples(timeMat, posMat, oriMat);

	Mat features;
	getFeatureVec(features, timeMat, posMat, oriMat);

	Mat trainSet, testSet;
	getTrainTestSets(trainSet, testSet, features);

	char in;
	cin >> in;

	return 0;
}