/** 
* @author: Jacek Turula
* @file main.cpp
* This program is suppoused to display live video from the camera.
*/

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;

/**
* \fn Mat detectMotion(Mat& frame1, Mat& frame2, int threshold);
* \param frame 1 is the previous video frame
* \param frame 2 is the current video frame.
* \param threshold is the value to which the difference between the frames is compared.
* \return the returned variable is a video frame containing the motion map.
* \brief This function computes the motion map for the previous (frame1) and current (frame2) video frames.
* The difference between the two frames is computed and if it is greater than the specified threshold, the value of that pixel of the motion map
* is set to 255. Otherwise, it's set to 0.
*/
Mat detectMotion(Mat& frame1, Mat& frame2, int threshold);

int main()
{
	/// VARIABLES
	VideoCapture capture; /// This variable captures the video stream from the camera.
	capture.open(0);	/// The video stream variable is initialized with the default ('0') camera device on the PC.

	Mat templateEmpty; /// This image variable contains the video frame WITHOUT the player's body.
	Mat templatePlayer; /// This image variable contains the video frame WITH the player's body.
	Mat frame; ///Current frame image variable.
	Mat prevFrame; 
	Mat motionMap; /// Image variable that is the result of image detection.

	int threshold = 15;

	/// INITIAL CAMERA OPERATIONS
	namedWindow("Video");
	namedWindow("Difference Image");
	capture >> frame;
	prevFrame = frame.clone(); /// The previous frame is initialized with the current variable before entering the while loop.
	waitKey(10);  /// A wait is performed so that the camera has enough time to start working.

	/// PROCESSING LOOP
	while (1)
	{
		capture >> frame; /// Current frame is captured and displayed. WARNING: THIS APPROACH CAUSES THE FIRST VIDEO FRAME TO NOT BE DISPLAYED!
		if (!(frame.empty())) /// The operations are conducted only if the captured video frame is not empty!
		{
			imshow("Video", frame);
			motionMap = detectMotion(prevFrame, frame,threshold);
			imshow("Difference Image", motionMap);
			prevFrame = frame.clone(); /// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.
			waitKey(10);
		}
	}
}

Mat detectMotion(Mat& frame1, Mat& frame2, int threshold)
{
	Mat frame1Gray, frame2Gray;
	int rows = frame1.rows;
	int cols = frame2.cols;
	int difference;

	Mat motionMap(rows, cols, CV_8UC1); /// The difference image is initialized with the same amount of rows and columns as the function's frames.
										/// 

	cvtColor(frame1, frame1Gray, CV_RGB2GRAY); /// Both previous and current frames are converted to the Grayscale color space so that the
											   /// computation of differential image is faster.
	cvtColor(frame2, frame2Gray, CV_RGB2GRAY);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			difference = abs(frame2Gray.at<uchar>(i, j) - frame1Gray.at<uchar>(i, j));
			if (difference > threshold)
				motionMap.at<uchar>(i, j) = 255;
			else
				motionMap.at<uchar>(i, j) = 0;
		}
	}
	return motionMap;
}