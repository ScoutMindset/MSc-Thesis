/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version. This program was written based on the tutorial
** available on the following site http://learnopengl.com/#!In-Practice/2D-Game/Breakout
******************************************************************/
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include < stdio.h>  
#include < iostream>  

#include < opencv2\opencv.hpp>  
#include < opencv2/core/core.hpp>  
#include < opencv2/video/tracking.hpp>
#include < opencv2/highgui/highgui.hpp>  
#include < opencv2/video/background_segm.hpp>  
#include <windows.h>

#include <time.h>

#include "game.h"
#include "resource_manager.h"


// GLFW function declerations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// The Width of the screen
const GLuint SCREEN_WIDTH = 640;
// The height of the screen
const GLuint SCREEN_HEIGHT = 480;

Game Breakout(SCREEN_WIDTH, SCREEN_HEIGHT);

void initializeTexture(GLuint *texture);

// OpenCV part #1
using namespace std;
using namespace cv;

/// MORPHOLOGICAL VARIABLES
int morph_elem = 0;
int morph_size = 0;
int morph_operator = 0;
int const max_operator = 4;
int const max_elem = 2;
int const max_kernel_size = 21;

///MOTION DETECTION VARIABLES
Mat motionMap; /// Image variable that is the result of image detection.
Mat motionMapMorph; /// Image variable that is the result of image detection after morphological operations.

///OTHER GLOBAL VARIABLES
int MAX_KERNEL_LENGTH = 20;
char filter;
char smoothing;

Mat detectMotion(Mat& frame1, Mat& frame2, int threshold, char colorSpace);

void modelGaussianBackground(VideoCapture &capture, Mat &meanValue, Mat &standardDeviation);

void Morphology_Operations(int, void*);
// OpenCV end of part #1
int main(int argc, char *argv[])
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError(); // Call it once to catch glewInit() bug, all other errors are now from our application.

	glfwSetKeyCallback(window, key_callback);

	// OpenGL configuration
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize game
	Breakout.Init();

	// DeltaTime variables
	GLfloat deltaTime = 0.0f;
	GLfloat lastFrame = 0.0f;

	// Start Game within Menu State
	Breakout.State = GAME_ACTIVE;

	// OpenCV part #2
	/// VARIABLES
	HWND consoleWindow = GetConsoleWindow();
	MoveWindow(consoleWindow, 10, 10, 800, 480, FALSE);
	VideoCapture capture; /// This variable captures the video stream from the camera.
	capture.open(0);	/// The video stream variable is initialized with the default ('0') camera device on the PC.	
	waitKey(10);  /// A wait is performed so that the camera has enough time to start working.

	Mat frame, finalFrame; ///Current frame image variable.
	Mat frameSmooth;
	Mat frameSmall;
	Mat prevFrame;
	Mat meanValue = Mat::zeros(SCREEN_WIDTH, SCREEN_HEIGHT, CV_16UC3);
	Mat standardDeviation = Mat::zeros(SCREEN_WIDTH, SCREEN_HEIGHT, CV_16UC3);

	vector<vector<Point> > contours;
	Scalar color(0, 0, 255);
	vector<Vec4i> hierarchy;

	int threshold = 17;
	int delay = 0;
	int k = -1;
	int clockIterator = 0;
	int resizeScale = 3;

	char colorSpace;

	clock_t t;
	clock_t sum = 0;


	string window_morph_name = "Difference Image After Morphological Operations";

	bool disablePlayerTemplate = 1;

	signed char input;

	/// INITIAL CAMERA OPERATIONS
	namedWindow("Video");
	moveWindow("Video", 10, 400);
	namedWindow("Difference Image", WINDOW_NORMAL);
	moveWindow("Difference Image", 600, 400);

	std::cout << "Please specify if you want to apply smoothing: (y)es or (n)o." << std::endl;
	std::cin >> smoothing;
	if ((smoothing == 'y') || (smoothing == 'Y'))
	{
		std::cout << "Please specify which filter you want to apply to the current frame: (h)omogenous, (g)aussian, (m)edian or (b)ilateral." << std::endl;
		std::cin >> filter;
	}

	std::cout << "Please specify to which space should the background model be transformed: (g)rayscale, (h)sv or (r)gb.";
	std::cin >> colorSpace;

	capture >> frame;
	finalFrame = frame.clone();
	Mat templateEmpty = Mat::zeros(frame.size(), CV_8UC3); /// This image variable contains the video frame WITHOUT the player's body.
	flip(frame, frame, 1); /// This function causes the mirror-like display of the video from the camera.
	cv::resize(frame, frameSmall, Size(frame.size().width / resizeScale, frame.size().height / resizeScale));
	


	///SMOOTHING OF THE INITIAL FRAME
	if (smoothing == 'n')
		prevFrame = frame.clone(); /// The previous frame is initialized with the current variable before entering the while loop.
	else
	{
		if ((filter == 'H') || (filter == 'h'))
		{
			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
				blur(frameSmall, prevFrame, Size(i, i), Point(-1, -1));
		}
		else if ((filter == 'G') || (filter == 'g'))
		{
			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
				GaussianBlur(frameSmall, prevFrame, Size(i, i), 0, 0);
		}
		else if ((filter == 'M') || (filter == 'm'))
		{
			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
				medianBlur(frameSmall, prevFrame, i);
		}
		else if ((filter == 'B') || (filter == 'b'))
		{
			for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
				bilateralFilter(frameSmall, prevFrame, i, i * 2, i / 2);
		}
	}


	/// Create window 
	namedWindow(window_morph_name, WINDOW_NORMAL);
	moveWindow(window_morph_name, 1250, 10);

	/// Create Trackbar to select Morphology operation
	createTrackbar("Operator:\n 0: Opening - 1: Closing  \n 2: Gradient - 3: Top Hat \n 4: Black Hat",
		"Video", &morph_operator, max_operator, Morphology_Operations);

	/// Create Trackbar to select kernel type
	createTrackbar("Element:\n 0: Rect - 1: Cross - 2: Ellipse", "Video",
		&morph_elem, max_elem,
		Morphology_Operations);

	/// Create Trackbar to choose kernel size
	createTrackbar("Kernel size:\n 2n +1", "Video",
		&morph_size, max_kernel_size,
		Morphology_Operations);

	/// PROCESSING LOOP
	cout << "Press ESCAPE in order to leave the program." << endl;

	// End of OpenCV part #2

	while (!glfwWindowShouldClose(window))
	{

		// OpenCV part #3


		capture >> frame; /// Current frame is captured and displayed. WARNING: THIS APPROACH CAUSES THE FIRST VIDEO FRAME TO NOT BE DISPLAYED
		/// BECAUSE THE FIRST FRAME IS CAPTURED OUTSIDE OF THE WHILE LOOP!

		if (!(frame.empty())) /// The operations are conducted only if the captured video frame is not empty!
		{
			flip(frame, frame, 1);
			cv::resize(frame, frameSmall, Size(frame.size().width / resizeScale, frame.size().height / resizeScale));
			if (smoothing == 'y')
			{
				t = clock();
				///SMOOTHING OF THE CURRENT FRAME
				if ((filter == 'H') || (filter == 'h'))
				{
					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
						blur(frameSmall, frameSmooth, Size(i, i), Point(-1, -1));
				}
				else if ((filter == 'G') || (filter == 'g'))
				{
					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
						GaussianBlur(frameSmall, frameSmooth, Size(i, i), 0, 0);
				}
				else if ((filter == 'M') || (filter == 'm'))
				{
					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
						medianBlur(frameSmall, frameSmooth, i);
				}
				else if ((filter == 'B') || (filter == 'b'))
				{
					for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
						bilateralFilter(frameSmall, frameSmooth, i, i * 2, i / 2);
				}

				t = clock() - t;
				sum += t;
				if (clockIterator == 100)
				{
					std::cout << "Average time of smoothing per frame is: " << double(sum) / (CLOCKS_PER_SEC * 100) << " seconds" << std::endl;
					sum = 0;
					clockIterator = 0;
					if (k == 1)
						cv::imshow("Empty template", prevFrame);
				}
				clockIterator++;

				/// MOTION DETECTION BEGINS
				t = clock();
				motionMap = detectMotion(prevFrame, frameSmooth, threshold, colorSpace);
				if (k < 1)
					prevFrame = frameSmooth.clone();/// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.

			}
			else
			{
				/// MOTION DETECTION BEGINS
				t = clock();
				motionMap = detectMotion(prevFrame, frameSmall, threshold, colorSpace);
				if (k < 1)
					prevFrame = frameSmall.clone();/// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.

			}

			if (clockIterator == 100)
			{
				t = clock() - t;
				std::cout << "Time of detecting motion per frame is: " << double(t) / (CLOCKS_PER_SEC) << " seconds" << std::endl;
			}


			Morphology_Operations(0, 0);///Morphological operations are done here;
			//imshow("Difference Image", motionMap); ///This shows the difference between the current and previous frames, but after the background template is 
			///captured the difference is computed between the current frame and the frame with just the background.

			/// INTERFACE OF FRAME-CAPTURING 
			/**
			* k == -1 (the program invites the user to capture the video frame without his/her body)
			* k == 0  (the program captures the frame without the user's body)
			*/

			if (k == -1)
			{
				cout << "Please ENTER and remove yourself from the frame to capture the frame with the background ONLY." << endl;
				k = 0;

			}

			input = waitKey(30); /// \var input This variable determines whether to capture the frame ("ENTER") or leave the program ("ESCAPE")

			if ((input == 13) && (k == 0))
			{
				delay = 50; /// The instance of ENTER-pressing causes the activation of frame-capturing delay so that the player can leave
				/// the frame so that frame WITHOUT the player's body is captured.

			}
			else if (input == 27)
				break;
			if (delay != 0) /// This part of the code is responsible for the countdown that allows the user to remove her/himself from the frame.
				/// WARNING: THIS COUNTDOWN IS NOT IN SECONDS AND DEPENDS ON HOW FAST THE REST OF THE WHILE LOOP IS EXECUTED!
			{
				if ((delay % 10) == 0)
					cout << delay / 10 << endl;
				delay--;
				if (delay == 0)
				{
					//if (smoothing == 'y')
					//prevFrame = frameSmooth.clone();
					//else
					//prevFrame = frame.clone(); ///The previous frame is constant from this point and is always equal to the empty background frame.
					k = 1;
					namedWindow("Empty template");
					moveWindow("Empty template", 900, 10);
					cv::imshow("Empty template", prevFrame);
				}
			}

			cv::imshow("Difference Image", motionMap);
			
			cv::resize(motionMapMorph, motionMapMorph, Size(frame.size().width, frame.size().height));
			cv::findContours(motionMapMorph.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));///This functions finds the countours in
			///the motion map after morpological											
			///operations have been conducted upon it.
			/// CV_RETR_EXTERNAL
			/// CV_RETR_LIST
			/// CV_RETR_CCOMP
			/// CV_RETR_TREE
			cv::resize(frame, frame, Size(motionMapMorph.size().width, motionMapMorph.size().height));
			
			for (int i = 0; i < contours.size(); i++)
				drawContours(frame, contours, i, color, 1, 8, hierarchy, 0, Point());
			//cutPlayer(frame, motionMapMorph);
			cv::resize(frame, finalFrame, Size(finalFrame.size().width, finalFrame.size().height));
			
			
		}
		cv::imshow(window_morph_name, motionMapMorph);
		// End of OpenCV part #3
		Breakout.currentFrame = finalFrame;
		
		Breakout.playerFrame = motionMapMorph;
		// Calculate delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		//deltaTime = 0.001f;
		// Manage user input
		Breakout.ProcessInput(deltaTime);

		// Update Game state
		Breakout.Update(deltaTime);

		// Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		Breakout.Render(frame);

		glfwSwapBuffers(window);
		cv::imshow("Video", Breakout.currentFrame);
	}
	destroyAllWindows();
	// Delete all resources as loaded using the resource manager
	ResourceManager::Clear();
	//glfwTerminate(); // When the program runs for an extended period of time (1 minute and more) this command crashes it.
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			Breakout.Keys[key] = GL_TRUE;
		else if (action == GLFW_RELEASE)
		{
			Breakout.Keys[key] = GL_FALSE;
			//Breakout.KeysProcessed[key] = GL_FALSE;
		}
	}
}
// OpenCV part #4
Mat detectMotion(Mat& frame1, Mat& frame2, int threshold, char colorSpace)
{
	Mat  frame1Trans;
	Mat  frame2Trans;
	double a = 0.001;
	int rows = frame1.rows;
	int cols = frame2.cols;
	int channel[3];
	int difference = 0, channels;

	Mat motionMap(rows, cols, CV_8UC1); /// The difference image is initialized with the same amount of rows and columns as the function's frames.
	/// 
	if ((colorSpace == 'g') || (colorSpace == 'G'))
	{
		cvtColor(frame1, frame1Trans, CV_BGR2GRAY); /// Both previous and current frames are converted to the Grayscale color space so that the
		/// computation of differential image is faster.
		cvtColor(frame2, frame2Trans, CV_BGR2GRAY);// Should it be RGB or BGR???
		channels = 1;
	}
	else if ((colorSpace == 'h') || (colorSpace == 'H'))
	{
		cvtColor(frame1, frame1Trans, CV_BGR2HSV); /// Both previous and current frames are converted to the Grayscale color space so that the
		/// computation of differential image is faster.
		cvtColor(frame2, frame2Trans, CV_BGR2HSV);// Should it be RGB or BGR???
		channels = 2;
	}
	else if ((colorSpace == 'r') || (colorSpace == 'R'))
	{
		frame1Trans = frame1.clone();
		frame2Trans = frame2.clone();
		channels = 3;
	}

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			for (int k = 0; k < channels; k++)
			{
				/*difference += (frame2Trans.at<Vec3b>(i, j)[k] - frame1Trans.at<Vec3b>(i, j)[k]) *
					(frame2Trans.at<Vec3b>(i, j)[k] - frame1Trans.at<Vec3b>(i, j)[k]);*/
				if (channels >1)
					difference += abs(frame2Trans.at<Vec3b>(i, j)[k] - frame1Trans.at<Vec3b>(i, j)[k]);
				else 
					difference += abs(frame2Trans.at<uchar>(i, j) - frame1Trans.at<uchar>(i, j));
			}
			

			/*difference/channels;*/
			/*sqrt(difference)*/
			if (difference / channels> threshold)
			{
				motionMap.at<uchar>(i, j) = 255;
			}
			else
			{
				motionMap.at<uchar>(i, j) = 0;
				frame1.at<Vec3b>(i, j)[0] = 0.99*frame1.at<Vec3b>(i, j)[0] + 0.01*frame2.at<Vec3b>(i, j)[0];
				frame1.at<Vec3b>(i, j)[1] = 0.99*frame1.at<Vec3b>(i, j)[1] + 0.01*frame2.at<Vec3b>(i, j)[1];
				frame1.at<Vec3b>(i, j)[2] = 0.99*frame1.at<Vec3b>(i, j)[2] + 0.01*frame2.at<Vec3b>(i, j)[2];
			}
			difference = 0;
		}
	}
	return motionMap;
}

Mat detectMotionGaussian(Mat& currentFrame, Mat& meanValue, Mat& standardDeviation, int threshold)
{
	Mat  frame1Trans;
	Mat  frame2Trans;
	double a = 0.001;
	int difference = 0, channels;

	Mat motionMap(currentFrame.rows, currentFrame.cols, CV_8UC1); /// The difference image is initialized with the same amount of rows and columns as the function's frames.

	for (int i = 0; i < currentFrame.rows; i++)
	{
		for (int j = 0; j < currentFrame.cols; j++)
		{
			for (int k = 0; k < channels; k++)
			{
				difference += abs(currentFrame.at<Vec3b>(i, j)[k] - meanValue.at<Vec3b>(i, j)[k] - standardDeviation.at<Vec3b>(i, j)[k]);
			}


			/*difference/channels;*/
			/*sqrt(difference)*/
			if (difference / channels> threshold)
			{
				motionMap.at<uchar>(i, j) = 255;
			}
			else
			{
				motionMap.at<uchar>(i, j) = 0;
				/*frame1.at<Vec3b>(i, j)[0] = 0.999*frame1.at<Vec3b>(i, j)[0] + 0.001*frame2.at<Vec3b>(i, j)[0];
				frame1.at<Vec3b>(i, j)[1] = 0.999*frame1.at<Vec3b>(i, j)[1] + 0.001*frame2.at<Vec3b>(i, j)[1];
				frame1.at<Vec3b>(i, j)[2] = 0.999*frame1.at<Vec3b>(i, j)[2] + 0.001*frame2.at<Vec3b>(i, j)[2];*/
			}
			difference = 0;
		}
	}
	return motionMap;
}

void modelGaussianBackground(VideoCapture &capture, Mat &meanValue, Mat &standardDeviation)
{
	Mat matArray[100];
	for (int i = 0; i < 100; i++)
	{
		capture >> matArray[i];
		add(meanValue, matArray[i], meanValue);
	}
	meanValue /= 100;

	for (int i = 0; i < meanValue.rows; i++)
	{
		for (int j = 0; j < meanValue.cols; j++)
		{
			for (int k = 0; k < 100; k++)
			{
				standardDeviation.at<Vec3d>(i, j)[0] += (meanValue.at<Vec3b>(i, j)[0] - matArray[k].at<Vec3b>(i, j)[0]) *
					(meanValue.at<Vec3b>(i, j)[0] - matArray[k].at<Vec3b>(i, j)[0]);
				standardDeviation.at<Vec3d>(i, j)[1] += (meanValue.at<Vec3b>(i, j)[1] - matArray[k].at<Vec3b>(i, j)[1]) *
					(meanValue.at<Vec3b>(i, j)[1] - matArray[k].at<Vec3b>(i, j)[1]);
				standardDeviation.at<Vec3d>(i, j)[2] += (meanValue.at<Vec3b>(i, j)[2] - matArray[k].at<Vec3b>(i, j)[2]) *
					(meanValue.at<Vec3b>(i, j)[2] - matArray[k].at<Vec3b>(i, j)[2]);
			}
		}
	}
	standardDeviation /= 100;
}


//Mat detectMotion1G(Mat& frame, Mat& mean, Mat& cov)
//{
//	
//}


void Morphology_Operations(int, void*)
{

	// Since MORPH_X : 2,3,4,5 and 6
	int operation = morph_operator + 2;

	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));

	/// Apply the specified morphology operation
	morphologyEx(motionMap, motionMapMorph, operation, element);


}
// End of OpenCV part #4