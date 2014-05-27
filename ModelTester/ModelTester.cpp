#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5
#define TEST_SET_FRACTION .25

using namespace cv;
using namespace std;

// Fill in sample mats with all the accumulated data.
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

// Extract features from raw accumulated sample data. Each row is a data from one frame of video.
// Time has 5 values, posMat has position data. each row is 20 joints, each joint has x, y, z, w where
// w is tracking state of joint. oriMat has orientation data, in w, x, y, z quaternion.
// Here is the most basic features, using the raw orientation data.
void getFeatureVec(Mat &features, const Mat &timeMat, const Mat &posMat, const Mat &oriMat) {
	oriMat.copyTo(features);
}

void getTrainTestSets(Mat &train, Mat &test, Mat &testLabels, Mat &trainLabels, Mat &featureSamples, Mat &labels) {
	Mat indices(featureSamples.size().height, 1, CV_32S);
	for (int i = 0; i < indices.size().height; ++i) {
		indices.at<int>(i) = i;
	}

	randShuffle(indices, 3);

	int cutoff = featureSamples.size().height * TEST_SET_FRACTION;
	Mat testIndices = indices.rowRange(0, cutoff); //TODO: be careful because this Mat header points to same data as indices
	Mat trainIndices = indices.rowRange(cutoff, featureSamples.size().height);
	/*cout << "height: " << testIndices.size().height << endl;
	cout << "height: " << trainIndices.size().height << endl;
	for (int i = 0; i < testIndices.size().height; ++i) {
		cout << testIndices.at<int>(i) << " ";
	}
	cout << endl;
	for (int i = 0; i < trainIndices.size().height; ++i) {
		cout << trainIndices.at<int>(i) << " ";
	}*/

	cout << "testIndices height: " << testIndices.size().height << endl;
	cout << "featureSamples height: " << featureSamples.size().height << endl;
	test.create(testIndices.size().height, featureSamples.size().width, CV_32F);
	testLabels.create(testIndices.size().height, 1, CV_32F);
	for (int i = 0; i < testIndices.size().height; ++i) {
		test.row(i) = (featureSamples.row(testIndices.at<int>(i)) + 0);
		testLabels.row(i) = (labels.row(testIndices.at<int>(i)) + 0);
		/*if (i < 100) {
			for (int j = 0; j < testLabels.size().width; ++j) {
				//cout << test.at<float>(i, j) << " ";
				cout << testLabels.at<float>(i, j) << " ";
			}
			cout << endl << endl;
		}*/
	}

	train.create(trainIndices.size().height, featureSamples.size().width, CV_32F);
	trainLabels.create(trainIndices.size().height, 1, CV_32F);
	for (int i = 0; i < trainIndices.size().height; ++i) {
		train.row(i) = (featureSamples.row(trainIndices.at<int>(i)) + 0); //TODO: use copyTo instead
		trainLabels.row(i) = (labels.row(trainIndices.at<int>(i)) + 0);
	}

}

void trainModel(Mat &trainSet, Mat &trainLabels, CvSVM &svm) { // TODO: consider combining train and test into one method, so can encapsulate even the cvstatmodel used
	// Set up SVM's parameters
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

	svm.train(trainSet, trainLabels, Mat(), Mat(), params);
}

void predictAll(Mat &testSet, Mat &testLabels, CvSVM &svm) {
	std::cout << endl << endl << "Prediction results: " << endl << endl;

	float prevClass = 0;
	float response = 0;
	
	Mat responses;
	//for (int i = 0; i < testSet.size().height; ++i) { // TODO: use testLabels to get classification rate
		//response = svm.predict(testSet.row(i));
		//printf("%2.0f, ", response);
	//}
	svm.predict(testSet, responses);
	for (int i = 0; i < responses.size().height; ++i) {
		cout << responses.at<float>(i) << " ";
	}

	cout << endl << endl;

	for (int i = 0; i < testLabels.size().height; ++i) {
		cout << testLabels.at<float>(i) << " ";
	}

	cout << endl << endl;

	Mat cmp(responses.size().height, responses.size().width, CV_32F);
	for (int i = 0; i < cmp.size().height; ++i) {
		cmp.at<float>(i) = (responses.at<float>(i) == testLabels.at<float>(i) ? 0 : 1);
		cout << cmp.at<float>(i) << " ";
	}

	Mat confusionMat = Mat::zeros(4, 4, CV_32S);
	for (int i = 0; i < responses.size().height; ++i) {
		confusionMat.at<int>(testLabels.at<float>(i), responses.at<float>(i))++;
	}
	cout << "confusion matrix: " << endl;
	for (int i = 0; i < 4; ++i) { //TODO: dont hardcode number of classes here
		for (int j = 0; j < 4; ++j) {
			printf("%2d ", confusionMat.at<int>(i, j));
		}
		cout << endl;
	}

	int numCorrect = 0;
	for (int i = 0; i < 4; ++i) { //TODO: dont hardcode number of classes here
		numCorrect += confusionMat.at<int>(i, i);
	}
	cout << "Correct: " << numCorrect << endl;
	cout << "Total: " << responses.size().height << endl;
	cout << "Accuracy: " << numCorrect * 1.0 / responses.size().height << endl;
}

int main(int, char**) {
	Mat timeMat, posMat, oriMat, labels;
	getSamples(timeMat, posMat, oriMat, labels);

	Mat features;
	getFeatureVec(features, timeMat, posMat, oriMat);

	Mat trainSet, testSet, testLabels, trainLabels;
	getTrainTestSets(trainSet, testSet, testLabels, trainLabels, features, labels);

	CvSVM svm;
	trainModel(trainSet, trainLabels, svm);

	predictAll(testSet, testLabels, svm);

	char in;
	cin >> in;

	return 0;
}