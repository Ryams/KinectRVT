// This program is meant to read in the raw sample data and analyze it in order to find the best features to use.
// This involves taking the covariance matrix, and maybe other things.

#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_TIME_DIMS 5

using namespace cv;
using namespace std;

// Extract raw accumulated sample data. Each row is a data from one frame of video.
// Time has 5 values, posMat has position data. each row is 20 joints, each joint has x, y, z, w where
// w is tracking state of joint. oriMat has orientation data, in w, x, y, z quaternion.
void getSamples(Mat &m1, Mat &m2, Mat &m3, Mat &labels) {
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
	fs.open("D:\\ThesisData\\" + exerciseTypeName + " CSVs\\all_labels.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> labels;
	fs.release();
}

int main(int, char**) {
	Mat timeMat, posMat, oriMat, labelMat;

	getSamples(timeMat, posMat, oriMat, labelMat);

	// 1: Count the number of 1s in the label mat
	// 2: create new mat with that many rows
	// 3: loop over samples and labels, for each index with a 1, insert that row into new mat
	// Viola! you have a mat with only samples from the class 1.
	// Repeat this process for each class label.
	// Get the variance and means for each of those single-class mats, and compare
	Mat counts(4, 1, CV_32S); //TODO: dont hardcode number of classes

	for (int i = 0; i < counts.size().height; ++i) {
		counts.at<int>(i) = countNonZero(labelMat == i);
		cout << "num class " << i << ": " << counts.at<int>(i) << endl;
	}

	Mat class0(0, oriMat.size().width, CV_32F); //TODO: dont hardcode number of classes
	Mat class1(0, oriMat.size().width, CV_32F);
	Mat class2(0, oriMat.size().width, CV_32F);
	Mat class3(0, oriMat.size().width, CV_32F);
	
	std::vector<Mat> classSamples;
	classSamples.push_back(class0); //TODO: dont hardcode number of classes
	classSamples.push_back(class1);
	classSamples.push_back(class2);
	classSamples.push_back(class3);

	for (int i = 0; i < labelMat.size().height; ++i) {
		classSamples.at(labelMat.at<float>(i)).push_back(oriMat.row(i)); //TODO: interchange between posMat and oriMat
	}
	for (int i = 0; i < 4; ++i) { //TODO: dont hardcode number of classes
		if (classSamples.at(i).size().height > 0) {
			cout << i << ": " << classSamples.at(i).size() << endl;
			//cout << i << ": " << classSamples.at(i).row(0) << endl;
			Mat cov, mean;
			calcCovarMatrix(classSamples.at(i), cov, mean, CV_COVAR_NORMAL | CV_COVAR_ROWS, CV_32F); //TODO: also find means and variances within classes

			cout << "Covariance matrix diagonal : " << endl;
			//cout << cov << endl;;
			for (int j = 0; j < cov.size().height; ++j) {
				printf("%-4.2f ", cov.at<float>(j, j));
			}
			cout << endl << "Means: " << endl;
			cout << mean << endl << endl;
		}
	}

	Mat allCov, allMean;
	calcCovarMatrix(oriMat, allCov, allMean, CV_COVAR_NORMAL | CV_COVAR_ROWS, CV_32F); //TODO: also find means and variances within classes

	cout << "Covariance matrix diagonal : " << endl;
	//cout << cov << endl;;
	for (int i = 0; i < allCov.size().height; ++i) {
		printf("%-4.2f ", allCov.at<float>(i, i));
	}
	cout << endl << "Means: " << endl;
	cout << allMean << endl;

	char in;
	cin >> in;

	return 0;
}