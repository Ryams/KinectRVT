// This program is meant to read in one persons data (RyanData) and use that exclusively for training
// the model, while using everyone elses data (ThesisData) for testing the model.

#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5
#define TEST_SET_FRACTION .25
#define NUM_TRIALS 5
#define KNN_VAL 10

using namespace cv;
using namespace std;

// Extract raw accumulated sample data. Each row is a data from one frame of video.
// Time has 5 values, posMat has position data. each row is 20 joints, each joint has x, y, z, w where
// w is tracking state of joint. oriMat has orientation data, in w, x, y, z quaternion.
// Enter 'r' for dirChar to get RyanData, anything else to get ThesisData
void getSamples(char dirChar, const String exerciseTypeName, Mat &m1, Mat &m2, Mat &m3, Mat &labels) {
	String _mainDirName;
	String _dirModifier;

	if (dirChar == 'r') {
		_mainDirName = "RyanData";
		_dirModifier = "";
	}
	else {
		_mainDirName = "ThesisData";
		_dirModifier = " CSVs";
	}

	//TODO: I had to hack this because there was a weird bug where
	// _exerciseTypeName was getting written over after calling FileStorage fs;
	const String mainDirName = String(_mainDirName);
	const String dirModifier = String(_dirModifier);

	FileStorage fs("D:\\" + mainDirName + "\\" + exerciseTypeName + dirModifier + "\\time_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m1;
	fs.open("D:\\" + mainDirName + "\\" + exerciseTypeName + dirModifier + "\\pos_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m2;
	fs.open("D:\\" + mainDirName + "\\" + exerciseTypeName + dirModifier + "\\ori_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m3;
	fs.open("D:\\" + mainDirName + "\\" + exerciseTypeName + dirModifier + "\\all_labels.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> labels;
	fs.release();
}

// Here is the most basic features, using the raw orientation data.
void getRawOriFeature(Mat &features, const Mat &timeMat, const Mat &posMat, const Mat &oriMat) {
	oriMat.copyTo(features);
}

// Here is the most basic features, using the raw position data.
void getRawPosFeature(Mat &features, const Mat &timeMat, const Mat &posMat, const Mat &oriMat) {
	posMat.copyTo(features);
}

// TODO: make sure taking the vector difference actually makes sense for quaternion data
void getOriVelocityFeature(Mat &features, const Mat &timeMat, const Mat &posMat, const Mat &oriMat) {
	features.create(oriMat.size().height - 1, oriMat.size().width, CV_32F); // Note height is -1 because interaction between rows.
	for (int i = 0; i < features.size().height; ++i) {
		features.row(i) = ((oriMat.row(i + 1) - oriMat.row(i)));// / (timeMat.at<float>(i + 1, 3) - timeMat.at<float>(i, 3)));
	}
	cout << "features: " << endl;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < features.size().width; ++j) {
			cout << features.at<float>(i, j) << " ";
		}
		cout << endl;
	}
}

void getPosVelocityFeature(Mat &features, const Mat &timeMat, const Mat &posMat, const Mat &oriMat) {
	features.create(posMat.size().height - 1, posMat.size().width, CV_32F); // Note height is -1 because interaction between rows.
	for (int i = 0; i < features.size().height; ++i) {
		features.row(i) = ((posMat.row(i + 1) - posMat.row(i)));// / (timeMat.at<float>(i + 1, 3) - timeMat.at<float>(i, 3)));
	}
	cout << "features: " << endl;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < features.size().width; ++j) {
			cout << features.at<float>(i, j) << " ";
		}
		cout << endl;
	}
}

// Train the SVM model.
void trainSVMModel(Mat &trainSet, Mat &trainLabels, CvSVM &svm) { // TODO: consider combining train and test into one method, so can encapsulate even the cvstatmodel used
	// Set up SVM's parameters
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::RBF; //TODO: note this changes during testing with different vals!
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

	svm.train(trainSet, trainLabels, Mat(), Mat(), params);
	//svm.train_auto(trainSet, trainLabels, Mat(), Mat(), params);
}

void trainKNNModel(Mat &trainSet, Mat &trainLabels, CvKNearest &knn) {
	knn.train(trainSet, trainLabels);
}

// Generates responses (predictions) for the test set and outputs results.
void predictSVM(Mat &testSet, Mat &testLabels, Mat &responses, CvSVM &svm) { //TODO: dont need test labels here
	svm.predict(testSet, responses);
}

void predictKNN(Mat &testSet, Mat &testLabels, Mat &responses, CvKNearest &knn) { //TODO: dont need test labels here
	knn.find_nearest(testSet, KNN_VAL, responses, Mat(), Mat());
}

void predictRandom(Mat &testSet, Mat &testLabels, Mat &responses) { // Only useful for arm curl, which has error classes 0, 1
	responses = Mat(testLabels.size().height, testLabels.size().width, CV_32F);
	for (int i = 0; i < testLabels.size().height; ++i) {
		responses.at<float>(i) = 0.0;
	}
}

void aggregateResults(Mat &responses, Mat &testLabels, Mat &confusionMat) {
	/*Mat cmp(responses.size().height, responses.size().width, CV_32F);
	for (int i = 0; i < cmp.size().height; ++i) {
		cmp.at<float>(i) = (responses.at<float>(i) == testLabels.at<float>(i) ? 0 : 1);
	}
	cout << cmp.t() << endl;*/

	//Mat confusionMat = Mat::zeros(4, 4, CV_32S);
	for (int i = 0; i < responses.size().height; ++i) {
		confusionMat.at<int>(testLabels.at<float>(i), responses.at<float>(i))++;
	}
}

void outputResults(Mat &confusionMat) {
	cout << endl << "confusion matrix: " << endl;
	cout << confusionMat << endl;

	int numCorrect = 0;
	for (int i = 0; i < 4; ++i) { //TODO: dont hardcode number of classes here
		numCorrect += confusionMat.at<int>(i, i); // TODO: wrong
	}

	cout << "Correct: " << numCorrect << endl; // TODO: wrong
	cout << "Total: " << sum(confusionMat)[0] << endl; // TODO: wrong
	cout << "Accuracy: " << numCorrect * 1.0 / sum(confusionMat)[0] << endl;
}

String getExerciseTypeName() {
	String _exerciseTypeName;
	char exerciseType = 'j';
	std::cout << "What type of exercise? 'u' arm curl, 'i' arm circle, 'j' jumping jack" << endl;
	std::cin >> exerciseType;

	if (exerciseType == 'j') {
		_exerciseTypeName = "Jumping jack";
	}
	else if (exerciseType == 'u') {
		_exerciseTypeName = "Arm curl";
	}
	else {
		_exerciseTypeName = "Arm circle";
	}
	return _exerciseTypeName;
}

// Get samples and labels from xml, then extract features,
// shuffle and split the samples randomly intro training and test sets,
// train the model, and predict outcomes from the test set, outputting classification rates.
int main(int, char**) {
	const String exerciseTypeName = String(getExerciseTypeName());

	Mat trainTimeMat, trainPosMat, trainOriMat, trainLabels, trainFeatures;
	getSamples('t', exerciseTypeName, trainTimeMat, trainPosMat, trainOriMat, trainLabels);
	getRawOriFeature(trainFeatures, trainTimeMat, trainPosMat, trainOriMat);

	Mat testTimeMat, testPosMat, testOriMat, testLabels, testFeatures;
	getSamples('r', exerciseTypeName, testTimeMat, testPosMat, testOriMat, testLabels);
	getRawOriFeature(testFeatures, testTimeMat, testPosMat, testOriMat);

	CvSVM svm;
	trainSVMModel(trainFeatures, trainLabels, svm);

	//TODO: make it an input which model to use (KNN, SVM, etc.)
	//CvKNearest knn;
	//trainKNNModel(trainFeatures, trainLabels, knn);

	Mat responses;
	//predictRandom(testFeatures, testLabels, responses);
	predictSVM(testFeatures, testLabels, responses, svm);
	//predictKNN(testFeatures, testLabels, responses, knn);

	Mat confusionMat = Mat::zeros(4, 4, CV_32S);
	aggregateResults(responses, testLabels, confusionMat);

	outputResults(confusionMat);

	char in;
	cin >> in;

	return 0;
}
