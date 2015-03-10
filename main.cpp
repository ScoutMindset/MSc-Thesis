/** 
* @author: Jacek Turula
* This program is suppoused to display live video from the camera.
*/

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;

Mat detectMotion(Mat& frame1, Mat& frame2);

int main()
{
	VideoCapture capture;
	capture.open(0);
	Mat frame, prevFrame, motionMap;
	namedWindow("Video");
	namedWindow("Difference Image");
	capture >> frame;
	prevFrame = frame.clone();
	waitKey(10);
	while (1)
	{
		capture >> frame;
		if (!(frame.empty())){
			imshow("Video", frame);
			motionMap = detectMotion(prevFrame, frame);
			imshow("Difference Image", motionMap);
		}
		prevFrame = frame.clone();
		waitKey(10);
		
		
	}
}

Mat detectMotion(Mat& frame1, Mat& frame2)
{
	Mat frame1Gray, frame2Gray;
	int rows = frame1.rows;
	int cols = frame2.cols;
	int difference;
	Mat motionMap(rows, cols, CV_8UC1);

	cvtColor(frame1, frame1Gray, CV_RGB2GRAY);
	cvtColor(frame2, frame2Gray, CV_RGB2GRAY);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			difference = abs(frame2Gray.at<uchar>(i, j) - frame1Gray.at<uchar>(i, j));
			if (difference > 15)
				motionMap.at<uchar>(i, j) = 255;
			else
				motionMap.at<uchar>(i, j) = 0;
		}
	}
	return motionMap;
}