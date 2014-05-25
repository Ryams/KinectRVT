#include "opencv2/opencv.hpp"

#define NUM_JOINT_DIMS 4
#define NUM_SKEL_JOINTS 20
#define NUM_SKEL_TRACKED 6
#define NUM_TIME_DIMS 5
#define FRAME_CAP 1000
#define IMZISE 300

void displayJoints(float *positions);
