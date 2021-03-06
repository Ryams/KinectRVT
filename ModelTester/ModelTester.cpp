// This program is useful for retrieving a sample set and splitting it (shuffle and split)
// as well as running that process for multiple iterations to get an average confusion matrix
// for a particular exercise and statistical model. For using RyanData for training, see new program.

#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>  
#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5
#define TEST_SET_FRACTION .25
#define NUM_TRIALS 5

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
	//TODO: not sure if this is relevant, may want to just make new program to train on my data, test on others
	/*String mainDirName;
	char mainDir;

	cout << "Enter which main directory to use. 'r' for RyanData, 't' for ThesisData: " << endl;
	std::cin >> mainDir;

	if (mainDir == 'r') {
		mainDirName = "RyanData";
	}
	else {
		mainDirName = "ThesisData";
	}*/

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

	FileStorage fs("D:\\ThesisData\\" + exerciseTypeName + "\\time_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m1;
	fs.open("D:\\ThesisData\\" + exerciseTypeName + "\\pos_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m2;
	fs.open("D:\\ThesisData\\" + exerciseTypeName + "\\ori_samples.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> m3;
	fs.open("D:\\ThesisData\\" + exerciseTypeName + "\\all_labels.xml", FileStorage::READ);
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

// Split the feature samples into training and test sets. This is done by randomizing indices for each row,
// and splitting it according to the test set fraction.
void getTrainTestSets(Mat &train, Mat &test, Mat &testLabels, Mat &trainLabels, Mat &featureSamples, Mat &labels) {
	Mat indices(featureSamples.size().height, 1, CV_32S);
	for (int i = 0; i < indices.size().height; ++i) {
		indices.at<int>(i) = i;
	}

	randShuffle(indices, 3);

	int cutoff = featureSamples.size().height * TEST_SET_FRACTION;
	Mat testIndices = indices.rowRange(0, cutoff); //TODO: be careful because this Mat header points to same data as indices
	Mat trainIndices = indices.rowRange(cutoff, featureSamples.size().height);

	test.create(testIndices.size().height, featureSamples.size().width, CV_32F);
	testLabels.create(testIndices.size().height, 1, CV_32F);
	for (int i = 0; i < testIndices.size().height; ++i) {
		test.row(i) = (featureSamples.row(testIndices.at<int>(i)) + 0);
		testLabels.row(i) = (labels.row(testIndices.at<int>(i)) + 0);
	}

	train.create(trainIndices.size().height, featureSamples.size().width, CV_32F);
	trainLabels.create(trainIndices.size().height, 1, CV_32F);
	for (int i = 0; i < trainIndices.size().height; ++i) {
		train.row(i) = (featureSamples.row(trainIndices.at<int>(i)) + 0); //TODO: use copyTo instead
		trainLabels.row(i) = (labels.row(trainIndices.at<int>(i)) + 0);
	}

}

// Train the model. For now, svm is being used but later more models will be added as well.
void trainSVMModel(Mat &trainSet, Mat &trainLabels, CvSVM &svm) { // TODO: consider combining train and test into one method, so can encapsulate even the cvstatmodel used
	// Set up SVM's parameters
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
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
	knn.find_nearest(testSet, 10, responses, Mat(), Mat());
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
	/*cout << endl << "confusion matrix: " << endl;
	cout << confusionMat << endl;

	int numCorrect = 0;
	for (int i = 0; i < 4; ++i) { //TODO: dont hardcode number of classes here
		numCorrect += confusionMat.at<int>(i, i); // TODO: wrong
	}
	cout << "Correct: " << numCorrect << endl; // TODO: wrong
	cout << "Total: " << responses.size().height << endl; // TODO: wrong
	cout << "Accuracy: " << numCorrect * 1.0 / responses.size().height << endl; // TODO: wrong*/
}

void outputResults(Mat &confusionMat) {
	cout << endl << "confusion matrix: " << endl;
	cout << confusionMat << endl;

	int numCorrect = 0;
	for (int i = 0; i < 4; ++i) { //TODO: dont hardcode number of classes here
		numCorrect += confusionMat.at<int>(i, i); // TODO: wrong
	}

	cout << endl << "Num trials: " << NUM_TRIALS << endl;
	cout << "Correct: " << numCorrect << endl; // TODO: wrong
	cout << "Total: " << sum(confusionMat)[0] << endl; // TODO: wrong
	cout << "Accuracy: " << numCorrect * 1.0 / sum(confusionMat)[0] << endl;
}

// Get samples and labels from xml, then extract features,
// shuffle and split the samples randomly intro training and test sets,
// train the model, and predict outcomes from the test set, outputting classification rates.
int main(int, char**) {
	Mat timeMat, posMat, oriMat, labels;
	getSamples(timeMat, posMat, oriMat, labels);
	Mat confusionMat = Mat::zeros(4, 4, CV_32S);

	for (int i = 0; i < NUM_TRIALS; ++i) {
		Mat features;
		getRawOriFeature(features, timeMat, posMat, oriMat);

		Mat trainSet, testSet, testLabels, trainLabels;
		getTrainTestSets(trainSet, testSet, testLabels, trainLabels, features, labels);

		//CvSVM svm;
		//trainSVMModel(trainSet, trainLabels, svm);

		CvKNearest knn;
		trainKNNModel(trainSet, trainLabels, knn);

		Mat responses;
		//predictSVM(testSet, testLabels, responses, svm);
		predictKNN(testSet, testLabels, responses, knn);

		
		aggregateResults(responses, testLabels, confusionMat);
	}

	outputResults(confusionMat);

	char in;
	cin >> in;

	return 0;
}