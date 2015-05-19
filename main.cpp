///**
//* @author: Jacek Turula
//* @file main.cpp
//* This program is suppoused to display live video from the camera. It was written with the help of the following OpenCV tutorials:
//* http://docs.opencv.org/doc/tutorials/imgproc/opening_closing_hats/opening_closing_hats.html
//* http://docs.opencv.org/doc/tutorials/imgproc/shapedescriptors/find_contours/find_contours.html
//*/
//
//#include < stdio.h>  
//#include < iostream>  
//
//#include < opencv2\opencv.hpp>  
//#include < opencv2/core/core.hpp>  
//#include < opencv2/video/tracking.hpp>
//#include < opencv2/highgui/highgui.hpp>  
//#include < opencv2/video/background_segm.hpp>  
//
//
//#include <windows.h>
//
//#include <time.h>
//
//
////GRAPHICS
//#define GLEW_STATIC
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//
//#include "game.h"
//#include "resource_manager.h"
//
//
//
//using namespace std;
//using namespace cv;
//
///**
//* \fn Mat detectMotion(Mat& frame1, Mat& frame2, int threshold);
//* \param frame 1 is the previous video frame
//* \param frame 2 is the current video frame.
//* \param threshold is the value to which the difference between the frames is compared.
//* \return the returned variable is a video frame containing the motion map.
//* \brief This function computes the motion map for the previous (frame1) and current (frame2) video frames.
//* The difference between the two frames is computed and if it is greater than the specified threshold, the value of that pixel of the motion map
//* is set to 255. Otherwise, it's set to 0.
//*/
//
///// MORPHOLOGICAL VARIABLES
//int morph_elem = 0;
//int morph_size = 0;
//int morph_operator = 0;
//int const max_operator = 4;
//int const max_elem = 2;
//int const max_kernel_size = 21;
//
/////MOTION DETECTION VARIABLES
//Mat motionMap; /// Image variable that is the result of image detection.
//Mat motionMapMorph; /// Image variable that is the result of image detection after morphological operations.
//
/////OTHER GLOBAL VARIABLES
//int MAX_KERNEL_LENGTH = 15;
//char filter;
//char smoothing;
//
//Mat detectMotion(Mat& frame1, Mat& frame2, int threshold);
//
//void cutPlayer(Mat& frame1, Mat& frame2);
//
//void Morphology_Operations(int, void*);
//
//int main()
//{
//	/// VARIABLES
//	HWND consoleWindow = GetConsoleWindow();
//	MoveWindow(consoleWindow, 10, 10, 800, 480, FALSE);
//	VideoCapture capture; /// This variable captures the video stream from the camera.
//	capture.open(0);	/// The video stream variable is initialized with the default ('0') camera device on the PC.
//	
//	
//	Mat frame, finalFrame; ///Current frame image variable.
//	Mat frameSmooth;
//	Mat frameSmall;
//	Mat prevFrame;
//	vector<vector<Point> > contours;
//	Scalar color(0, 0, 255);
//	vector<Vec4i> hierarchy;
//
//	int threshold = 25;
//	int delay = 0;
//	int k = -1;
//	int clockIterator = 0;
//	int resizeScale = 3;
//
//	clock_t t;
//	clock_t sum=0;
//
//
//	string window_morph_name = "Difference Image After Morphological Operations";
//
//	bool disablePlayerTemplate = 1;
//
//	signed char input;
//
//	/// INITIAL CAMERA OPERATIONS
//	namedWindow("Video");
//	moveWindow("Video", 10, 400);
//	namedWindow("Difference Image", WINDOW_NORMAL);
//	moveWindow("Difference Image", 600, 400);
//
//	capture >> frame;
//	finalFrame = frame.clone();
//	Mat templateEmpty = Mat::zeros(frame.size(), CV_8UC3); /// This image variable contains the video frame WITHOUT the player's body.
//	flip(frame, frame, 1); /// This function causes the mirror-like display of the video from the camera.
//	resize(frame, frameSmall, Size(frame.size().width / resizeScale, frame.size().height / resizeScale));
//	waitKey(10);  /// A wait is performed so that the camera has enough time to start working.
//
//	std::cout << "Please specify if you want to apply smoothing: (y)es or (n)o." << std::endl;
//	std::cin >> smoothing;
//	if (smoothing == 'y')
//	{
//		std::cout << "Please specify which filter you want to apply to the current frame: (h)omogenous, (g)aussian, (m)edian or (b)ilateral." << std::endl;
//		std::cin >> filter;
//	}
//
//	///SMOOTHING OF THE INITIAL FRAME
//	if (smoothing == 'n')
//		prevFrame = frame.clone(); /// The previous frame is initialized with the current variable before entering the while loop.
//	else
//	{
//		if ((filter == 'H') || (filter == 'h'))
//		{
//			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//				blur(frameSmall, prevFrame, Size(i, i), Point(-1, -1));
//		}
//		else if ((filter == 'G') || (filter == 'g'))
//		{
//			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//				GaussianBlur(frameSmall, prevFrame, Size(i, i), 0, 0);
//		}
//		else if ((filter == 'M') || (filter == 'm'))
//		{
//			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//				medianBlur(frameSmall, prevFrame, i);
//		}
//		else if ((filter == 'B') || (filter == 'b'))
//		{
//			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//				bilateralFilter(frameSmall, prevFrame, i, i * 2, i / 2);
//		}
//	}
//
//
//	/// Create window 
//	namedWindow(window_morph_name, WINDOW_NORMAL);
//	moveWindow(window_morph_name, 1250, 10);
//
//	/// Create Trackbar to select Morphology operation
//	createTrackbar("Operator:\n 0: Opening - 1: Closing  \n 2: Gradient - 3: Top Hat \n 4: Black Hat",
//		"Video", &morph_operator, max_operator, Morphology_Operations);
//
//	/// Create Trackbar to select kernel type
//	createTrackbar("Element:\n 0: Rect - 1: Cross - 2: Ellipse", "Video",
//		&morph_elem, max_elem,
//		Morphology_Operations);
//
//	/// Create Trackbar to choose kernel size
//	createTrackbar("Kernel size:\n 2n +1", "Video",
//		&morph_size, max_kernel_size,
//		Morphology_Operations);
//
//	/// PROCESSING LOOP
//	cout << "Press ESCAPE in order to leave the program." << endl;
//
//	while (1)
//	{
//		
//		capture >> frame; /// Current frame is captured and displayed. WARNING: THIS APPROACH CAUSES THE FIRST VIDEO FRAME TO NOT BE DISPLAYED
//		/// BECAUSE THE FIRST FRAME IS CAPTURED OUTSIDE OF THE WHILE LOOP!
//		
//		if (!(frame.empty())) /// The operations are conducted only if the captured video frame is not empty!
//		{
//			flip(frame, frame, 1);
//			resize(frame, frameSmall, Size(frame.size().width / resizeScale, frame.size().height / resizeScale));
//			if (smoothing == 'y')
//			{
//				t = clock();
//				///SMOOTHING OF THE CURRENT FRAME
//				if ((filter == 'H') || (filter == 'h'))
//				{
//					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//						blur(frameSmall, frameSmooth, Size(i, i), Point(-1, -1));
//				}
//				else if ((filter == 'G') || (filter == 'g'))
//				{
//					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//						GaussianBlur(frameSmall, frameSmooth, Size(i, i), 0, 0);
//				}
//				else if ((filter == 'M') || (filter == 'm'))
//				{
//					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//						medianBlur(frameSmall, frameSmooth, i);
//				}
//				else if ((filter == 'B') || (filter == 'b'))
//				{
//					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
//						bilateralFilter(frameSmall, frameSmooth, i, i * 2, i / 2);
//				}
//
//				t = clock() - t;
//				sum += t;
//				if (clockIterator == 100)
//				{
//					std::cout << "Average time of smoothing per frame is: " << double(sum) / (CLOCKS_PER_SEC * 100) << " seconds" << std::endl;
//					sum = 0;
//					clockIterator = 0;
//					if (k==1)
//						imshow("Empty template", prevFrame);
//				}
//				clockIterator++;
//
//				/// MOTION DETECTION BEGINS
//				t = clock();
//				motionMap = detectMotion(prevFrame, frameSmooth, threshold);
//				if (k < 1)
//					prevFrame = frameSmooth.clone();/// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.
//					
//			}
//			else
//			{
//				/// MOTION DETECTION BEGINS
//				t = clock();
//				motionMap = detectMotion(prevFrame, frameSmall, threshold);
//				if (k < 1)
//					prevFrame = frameSmall.clone();/// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.
//					
//			}
//
//			if (clockIterator == 100)
//			{
//				t = clock() - t;
//				std::cout << "Time of detecting motion per frame is: " << double(t) / (CLOCKS_PER_SEC) << " seconds" << std::endl;
//			}
//
//
//			Morphology_Operations(0, 0);///Morphological operations are done here;
//			//imshow("Difference Image", motionMap); ///This shows the difference between the current and previous frames, but after the background template is 
//			///captured the difference is computed between the current frame and the frame with just the background.
//			
//			/// INTERFACE OF FRAME-CAPTURING 
//			/**
//			* k == -1 (the program invites the user to capture the video frame without his/her body)
//			* k == 0  (the program captures the frame without the user's body)
//			*/
//
//			if (k == -1)
//			{
//				cout << "Please ENTER and remove yourself from the frame to capture the frame with the background ONLY." << endl;
//				k = 0;
//
//			}
//
//			input = waitKey(30); /// \var input This variable determines whether to capture the frame ("ENTER") or leave the program ("ESCAPE")
//
//			if ((input == 13) && (k == 0))
//			{
//				delay = 50; /// The instance of ENTER-pressing causes the activation of frame-capturing delay so that the player can leave
//				/// the frame so that frame WITHOUT the player's body is captured.
//
//			}
//			else if (input == 27)
//				break;
//
//			if (delay != 0) /// This part of the code is responsible for the countdown that allows the user to remove her/himself from the frame.
//				/// WARNING: THIS COUNTDOWN IS NOT IN SECONDS AND DEPENDS ON HOW FAST THE REST OF THE WHILE LOOP IS EXECUTED!
//			{
//				if ((delay % 10) == 0)
//					cout << delay / 10 << endl;
//				delay--;
//				if (delay == 0)
//				{
//					//if (smoothing == 'y')
//						//prevFrame = frameSmooth.clone();
//					//else
//						//prevFrame = frame.clone(); ///The previous frame is constant from this point and is always equal to the empty background frame.
//					k = 1;
//					namedWindow("Empty template");
//					moveWindow("Empty template", 900, 10);
//					imshow("Empty template", prevFrame);
//				}
//			}
//
//			imshow("Difference Image", motionMap);
//			imshow(window_morph_name, motionMapMorph);
//			resize(motionMapMorph, motionMapMorph, Size(frame.size().width, frame.size().height));
//			findContours(motionMapMorph, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));///This functions finds the countours in
//																													 ///the motion map after morpological											
//																													 ///operations have been conducted upon it.
//																													 /// CV_RETR_EXTERNAL
//																													 /// CV_RETR_LIST
//																													 /// CV_RETR_CCOMP
//																													 /// CV_RETR_TREE
//			resize(frame, frame, Size(motionMapMorph.size().width, motionMapMorph.size().height));
//			for (int i = 0; i < contours.size(); i++)
//				drawContours(frame, contours, i, color, 1, 8, hierarchy, 0, Point());
//			cutPlayer(frame, motionMapMorph);
//			resize(frame, finalFrame, Size(finalFrame.size().width, finalFrame.size().height));
//			imshow("Video", finalFrame);
//
//		}
//
//	}
//	return 0;
//}
//
//void cutPlayer(Mat& frame1, Mat& frame2)
//{
//	for (int i = 0; i < frame1.rows; i++)
//	{
//		for (int k = 0; k < frame1.cols; k++)
//		{
//			if (frame2.at<uchar>(i, k) == 0)
//			{
//				frame1.at<Vec3b>(i, k)[0] = 0;
//				frame1.at<Vec3b>(i, k)[1] = 0;
//				frame1.at<Vec3b>(i, k)[2] = 0;
//			}
//		}
//	}
//}
//
//Mat detectMotion(Mat& frame1, Mat& frame2, int threshold)
//{
//	Mat  frame1Gray;
//	Mat  frame2Gray;
//	double a = 0.001;
//	int rows = frame1.rows;
//	int cols = frame2.cols;
//	int difference, difference1, difference2, difference3;
//
//	Mat motionMap(rows, cols, CV_8UC1); /// The difference image is initialized with the same amount of rows and columns as the function's frames.
//	/// 
//
//	cvtColor(frame1, frame1Gray, CV_RGB2GRAY); /// Both previous and current frames are converted to the Grayscale color space so that the
//	/// computation of differential image is faster.
//	cvtColor(frame2, frame2Gray, CV_RGB2GRAY);
//
//	for (int i = 0; i < rows; i++)
//	{
//		for (int j = 0; j < cols; j++)
//		{
//			//difference = abs(frame2Gray.at<uchar>(i, j) - frame1Gray.at<uchar>(i, j)); //d0
//			difference1 = abs(frame2.at<Vec3b>(i, j)[0] - frame1.at<Vec3b>(i, j)[0]);
//			difference2 = abs(frame2.at<Vec3b>(i, j)[1] - frame1.at<Vec3b>(i, j)[1]);
//			difference3 = abs(frame2.at<Vec3b>(i, j)[2] - frame1.at<Vec3b>(i, j)[2]);
//
//			/*difference = (difference1 + difference2 + difference3)/3;*/
//
//			difference = sqrt(difference1*difference1 + difference2*difference2 + difference3*difference3);
//			if (difference > threshold)
//			{
//				motionMap.at<uchar>(i, j) = 255;
//			}
//			else
//			{
//				motionMap.at<uchar>(i, j) = 0;		
//				/*frame1.at<Vec3b>(i, j)[0] = 0.999*frame1.at<Vec3b>(i, j)[0] + 0.001*frame2.at<Vec3b>(i, j)[0];
//				frame1.at<Vec3b>(i, j)[1] = 0.999*frame1.at<Vec3b>(i, j)[1] + 0.001*frame2.at<Vec3b>(i, j)[1];
//				frame1.at<Vec3b>(i, j)[2] = 0.999*frame1.at<Vec3b>(i, j)[2] + 0.001*frame2.at<Vec3b>(i, j)[2];*/
//			}
//			
//		}
//	}
//	return motionMap;
//}
//
////Mat detectMotion1G(Mat& frame, Mat& mean, Mat& cov)
////{
////	
////}
//
//
//void Morphology_Operations(int, void*)
//{
//
//	// Since MORPH_X : 2,3,4,5 and 6
//	int operation = morph_operator + 2;
//
//	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
//
//	/// Apply the specified morphology operation
//	morphologyEx(motionMap, motionMapMorph, operation, element);
//
//
//}
//
//
//
//
//
//
