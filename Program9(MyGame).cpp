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
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

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
int MAX_KERNEL_LENGTH = 10;
int resizeScale = 4;
char filter;
char smoothing = 'o'; //default values are different from the pool of choices available to the user so that the menu "while" loop is executed
static int currentCrosshairMeanRow = 0, currentCrosshairMeanCol = 0;
static int currentTriggerMeanRowRed = 0, currentTriggerMeanColRed = 0;
static int currentTriggerMeanRowGreen = 0, currentTriggerMeanColGreen = 0;

// DeltaTime variables (those are made global so that they can be used in the Breakout's "Update" function
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Mat detectMotion(Mat& frame1, Mat& frame2, int threshold, char update);//, char colorSpace);

Mat modelGaussianBackground(VideoCapture &capture, Mat &meanValue, char colorSpace, int resizeScale);

Mat detectMotionGaussian(Mat& currentFrame, Mat& meanValue, Mat& standardDeviation, int threshold, char colorSpace);

glm::vec2 handSimpleCrosshairControl(Mat& binaryMap, Mat& currentFrame, Mat& crosshairMap);

void handSimpleTriggerControl(Mat& binaryMap, Mat& currentFrame, Mat& triggerMap, glm::vec2& greenMean, glm::vec2& redMean);

void LabelComponent(unsigned short* STACK, unsigned short width, unsigned short height, unsigned char* input, 
	int* output, int labelNo, unsigned short x, unsigned short y);
void LabelImage(unsigned short width, unsigned short height, unsigned char* input, int* output);

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
	/*glfwSetCursorPosCallback(window, cursor_position_callback); //callbacks used to control the crosshair using the mouse
	glfwSetMouseButtonCallback(window, mouse_button_callback);*/

	// OpenGL configuration
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize game
	Breakout.Init();

	// Start Game within Menu State
	Breakout.State = GAME_ACTIVE;

	// OpenCV part #2
	/// VARIABLES
	HWND consoleWindow = GetConsoleWindow();
	MoveWindow(consoleWindow, 1100, 10, 800, 200, FALSE);
	VideoCapture capture; /// This variable captures the video stream from the camera.
	capture.open(0);	/// The video stream variable is initialized with the default ('0') camera device on the PC.	
	waitKey(100);  /// A wait is performed so that the camera has enough time to start working.

	int threshold;
	int delay = 0;
	int k = -1;
	int clockIterator = 0;
	int noBalls;
	int triggerStatus = 0;
	double triggerDistance;
	

	Mat frame, finalFrame; ///Current frame image variable.
	Mat frameSmooth;
	Mat frameSmall;
	Mat prevFrame;
	Mat meanValueGray = Mat::zeros(SCREEN_HEIGHT / resizeScale, SCREEN_WIDTH / resizeScale, CV_32FC1);
	Mat meanValueColor = Mat::zeros(SCREEN_HEIGHT / resizeScale, SCREEN_WIDTH / resizeScale, CV_32FC3);
	Mat meanValueImg, standardDeviationImg;
	Mat standardDeviationGray = Mat::zeros(SCREEN_HEIGHT / resizeScale, SCREEN_WIDTH / resizeScale, CV_32FC1);
	Mat standardDeviationColor = Mat::zeros(SCREEN_HEIGHT / resizeScale, SCREEN_WIDTH / resizeScale, CV_32FC3);
	Mat crosshairMap(1,1,CV_8UC3);
	Mat triggerMap(1, 1, CV_8UC3);
	Mat triggerMapLabelled(1, 1, CV_8UC3);

	vector<vector<Point> > contours;
	Scalar color(0, 0, 255);
	vector<Vec4i> hierarchy;

	glm::vec2 crosshairCorner = glm::vec2(0, 0);
	glm::vec2 greenMean = glm::vec2(0, 0);
	glm::vec2 redMean = glm::vec2(0, 0);

	char bsMethod = 'a'; //default values are different from the pool of choices available to the user so that the menu "while" loop is executed
	char colorSpace = 'p'; //default values are different from the pool of choices available to the user so that the menu "while" loop is executed

	clock_t t;
	clock_t sum = 0;


	string window_morph_name = "Difference Image After Morphological Operations";
	string window_crosshair_map_name = "Crosshair view";
	string window_trigger_map_name = "Trigger view";

	bool disablePlayerTemplate = 1;

	unsigned char input;
	unsigned char update;

	/// INITIAL CAMERA OPERATIONS
	namedWindow("Video");
	moveWindow("Video", 10, 400);
	namedWindow("Difference Image", WINDOW_NORMAL);
	moveWindow("Difference Image", 1100, 250);
	while ((bsMethod != 'c') && (bsMethod != 'C') && (bsMethod != 'g') && (bsMethod != 'G'))
	{
		std::cout << "Please specify if you want to use the (c)lassical BS method or the (G)aussian modelling method of BS" << std::endl;
		std::cin >> bsMethod;
	}
	std::cout << "Please specify the threshold for the BS method (the suggested value for the classical method is 15 and the" <<
				" suggested value for the Gaussian method is 10):" << std::endl;
	std::cin >> threshold;

	while ((smoothing != 'y') && (smoothing != 'Y') && (smoothing != 'n') && (smoothing != 'N') && 
		((bsMethod == 'c')||(bsMethod == 'C')))
	{

			std::cout << "Please specify if you want to apply smoothing: (y)es or (n)o." << std::endl;
			std::cin >> smoothing;
			std::cout << "Please specify if you want to update the background: (y)es or (n)o." << std::endl;
			std::cin >> update;
			//if ((smoothing == 'y') || (smoothing == 'Y'))
			//{
			//	std::cout << "Please specify which filter you want to apply to the current frame: (h)omogenous, (g)aussian, (m)edian or (b)ilateral." << std::endl;
			//	std::cin >> filter;
			//}

	}
	filter = 'g';

	if ((bsMethod == 'g') || (bsMethod == 'G'))
	{
		std::cout << "Please specify to which space should the background model be transformed: (g)rayscale, (H)SV, (Y)CrCb or (R)GB." << std::endl;
		std::cin >> colorSpace;
	}
	
	
	capture >> frame;
	cv::resize(crosshairMap, crosshairMap, Size(frame.size().width , frame.size().height));
	cv::resize(triggerMap, triggerMap, Size(frame.size().width, frame.size().height));
	finalFrame = frame.clone();
	Mat templateEmpty = Mat::zeros(frame.size(), CV_8UC3); /// This image variable contains the video frame WITHOUT the player's body.
	cv::flip(frame, frame, 1); /// This function causes the mirror-like display of the video from the camera.
	cv::resize(frame, frameSmall, Size(frame.size().width / resizeScale, frame.size().height / resizeScale));
	


	///SMOOTHING OF THE INITIAL FRAME
	if (smoothing == 'n')
		prevFrame = frameSmall.clone(); /// The previous frame is initialized with the current variable before entering the while loop.
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
	cv::moveWindow(window_morph_name, 1250, 10);

	namedWindow(window_crosshair_map_name, WINDOW_NORMAL);
	cv::moveWindow(window_crosshair_map_name, 1250, 500);

	namedWindow(window_trigger_map_name, WINDOW_NORMAL);
	cv::moveWindow(window_trigger_map_name, 650, 500);

	/// Create Trackbar to select Morphology operation
	cv::createTrackbar("Operator:\n 0: Opening - 1: Closing  \n 2: Gradient - 3: Top Hat \n 4: Black Hat",
		"Video", &morph_operator, max_operator, Morphology_Operations);

	/// Create Trackbar to select kernel type
	cv::createTrackbar("Element:\n 0: Rect - 1: Cross - 2: Ellipse", "Video",
		&morph_elem, max_elem,
		Morphology_Operations);

	/// Create Trackbar to choose kernel size
	cv::createTrackbar("Kernel size:\n 2n +1", "Video",
		&morph_size, max_kernel_size,
		Morphology_Operations);

	/// PROCESSING LOOP
	std::cout << "Press ESCAPE in order to leave the program." << endl;

	
	if ((bsMethod == 'g') || (bsMethod == 'G'))
	{
		delay = 500000000;
		while (1)
		{
			if (delay != 0) /// This part of the code is responsible for the countdown that allows the user to remove her/himself from the frame.
				/// WARNING: THIS COUNTDOWN IS NOT IN SECONDS AND DEPENDS ON HOW FAST THE REST OF THE WHILE LOOP IS EXECUTED!
			{
				if ((delay % 100000000) == 0)
					cout << delay / 100000000 << endl;
				delay--;
				if (delay == 0)
					break;
			}
		}
	}

	if (bsMethod=='g')
	{
		if ((colorSpace == 'g') || (colorSpace == 'G'))
		{
			standardDeviationGray = modelGaussianBackground(capture, meanValueGray, colorSpace, resizeScale);
			meanValueGray.convertTo(meanValueImg, CV_8UC1);
			standardDeviationGray.convertTo(standardDeviationImg, CV_8UC1);
			cv::imshow("Mean Value Background", meanValueImg);
			cv::imshow("Standard Deviation Background", standardDeviationImg);
		}
		else
		{
			standardDeviationColor = modelGaussianBackground(capture, meanValueColor, colorSpace, resizeScale);
			meanValueColor.convertTo(meanValueImg, CV_8UC3);
			standardDeviationColor.convertTo(standardDeviationImg, CV_8UC3);
			cv::imshow("Mean Value Background", meanValueImg);
			cv::imshow("Standard Deviation Background", standardDeviationImg);
		}
	}
	
	
	// End of OpenCV part #2
	
	while (!glfwWindowShouldClose(window))
	{

		// OpenCV part #3


		capture >> frame; /// Current frame is captured and displayed. WARNING: THIS APPROACH CAUSES THE FIRST VIDEO FRAME TO NOT BE DISPLAYED
		/// BECAUSE THE FIRST FRAME IS CAPTURED OUTSIDE OF THE WHILE LOOP!

		if (!(frame.empty())) /// The operations are conducted only if the captured video frame is not empty!
		{
			cv::flip(frame, frame, 1);
			cv::resize(frame, frameSmall, Size(frame.size().width / resizeScale, frame.size().height / resizeScale));
			if ((smoothing == 'y') || (smoothing == 'Y'))
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
				if (bsMethod == 'c' || bsMethod == 'C')
				{
					motionMap = detectMotion(prevFrame, frameSmooth, threshold,update);// , colorSpace);
					if (k < 1)
						prevFrame = frameSmooth.clone();/// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.
				}
					
				/*else if (bsMethod == 'g' || bsMethod == 'G')
				{
					if ((colorSpace == 'g') || (colorSpace == 'G'))
						motionMap = detectMotionGaussian(frameSmooth, meanValueGray, standardDeviationGray, threshold);
					else
						motionMap = detectMotionGaussian(frameSmooth, meanValueColor, standardDeviationColor, threshold);
				}*/
					
				

			}
			else
			{
				/// MOTION DETECTION BEGINS
				t = clock();
				if (bsMethod == 'c' || bsMethod == 'C')
				{
					motionMap = detectMotion(prevFrame, frameSmall, threshold,update);//, colorSpace);
					if (k < 1)
						prevFrame = frameSmall.clone();/// Current frame is assigned as the previous frame for the next iteration of the 'while' loop.
				}
				else if (bsMethod == 'g' || bsMethod == 'G')
				{
					if ((colorSpace == 'g') || (colorSpace == 'G'))
						motionMap = detectMotionGaussian(frameSmall, meanValueGray, standardDeviationGray, threshold, colorSpace);
					else
						motionMap = detectMotionGaussian(frameSmall, meanValueColor, standardDeviationColor, threshold, colorSpace);
				}
			}

			if (clockIterator == 100)
			{
				t = clock() - t;
				std::cout << "Time of detecting motion per frame is: " << double(t) / (CLOCKS_PER_SEC) << " seconds" << std::endl;
			}

			

			Morphology_Operations(0, 0);///Morphological operations are done here;
			//imshow("Difference Image", motionMap); ///This shows the difference between the current and previous frames, but after the background template is 
			///captured the difference is computed between the current frame and the frame with just the background.
			
			crosshairCorner = handSimpleCrosshairControl(motionMapMorph, frameSmall, crosshairMap);
			handSimpleTriggerControl(motionMapMorph, frameSmall, triggerMap,greenMean, redMean);
			Breakout.CursorPosition.x = crosshairCorner.x;
			Breakout.CursorPosition.y = crosshairCorner.y;
			Breakout.CursorUpdate();
			triggerDistance = glm::distance(greenMean, redMean);
			
			if (triggerDistance > 50)
			{
				triggerStatus = 1;
			}
				
			else if ((triggerDistance < 40) && (glm::length(greenMean)) && (glm::length(redMean))&&triggerStatus)
			{
				Breakout.CheckCrosshair();
				triggerStatus = 0;
			}
			imshow(window_crosshair_map_name, crosshairMap);
			imshow(window_trigger_map_name, triggerMap);
			
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
					
				}
			}
			if (k == 1)
			{
				namedWindow("Empty template");
				moveWindow("Empty template", 900, 10);
				cv::imshow("Empty template", prevFrame);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	Breakout.CursorPositionPrev.x = Breakout.CursorPosition.x;
	Breakout.CursorPositionPrev.y = Breakout.CursorPosition.y;
	Breakout.CursorPosition.x = xPos;
	Breakout.CursorPosition.y = yPos;
	Breakout.CursorUpdate();
	// When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
	{
		Breakout.CheckCrosshair();
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Breakout.CursorPositionPrev.x = Breakout.CursorPosition.x;
	Breakout.CursorPositionPrev.y = Breakout.CursorPosition.y;
	Breakout.CursorPosition.x = xpos;
	Breakout.CursorPosition.y = ypos;
	Breakout.CursorUpdate();
}

// OpenCV part #4
Mat modelGaussianBackground(VideoCapture &capture, Mat &meanValue, char colorSpace, int resizeScale)
{
	const int length = 250, channels = meanValue.channels();
	float alpha;
	Mat matElement, frame[length];
	for (int i = 0; i < length; i++)
	{
		capture >> matElement;
		cv::flip(matElement, matElement, 1);
		cv::resize(matElement, matElement, Size(matElement.size().width / resizeScale, matElement.size().height / resizeScale));
		if ((colorSpace == 'g') || (colorSpace == 'G'))
		{
			cvtColor(matElement, matElement, CV_BGR2GRAY);
			matElement.convertTo(frame[i], CV_32FC1);

		}
		else if ((colorSpace == 'h') || (colorSpace == 'H'))
		{
			cvtColor(matElement, matElement, CV_BGR2HSV);
			matElement.convertTo(frame[i], CV_32FC3);
		}
		else if ((colorSpace == 'y') || (colorSpace == 'Y'))
		{
			cvtColor(matElement, matElement, CV_BGR2YCrCb);
			matElement.convertTo(frame[i], CV_32FC3);
		}
		else if ((colorSpace == 'r') || (colorSpace == 'R'))
		{
			matElement.convertTo(frame[i], CV_32FC3);
		}

		add(meanValue, frame[i], meanValue);
	}
	meanValue /= length;

	Mat standardDeviationGray = Mat::zeros(SCREEN_HEIGHT / resizeScale, SCREEN_WIDTH / resizeScale, CV_32FC1);
	Mat standardDeviationColor = Mat::zeros(SCREEN_HEIGHT / resizeScale, SCREEN_WIDTH / resizeScale, CV_32FC3);
	if ((colorSpace == 'g') || (colorSpace == 'G'))
	{
		for (int i = 0; i < meanValue.rows; i++)
		{
			for (int j = 0; j < meanValue.cols; j++)
			{
				for (int k = 0; k < length; k++)
				{
					standardDeviationGray.at<float>(i, j) += (meanValue.at<float>(i, j) - frame[k].at<float>(i, j)) *
						(meanValue.at<float>(i, j) - frame[k].at<float>(i, j));
				}
			}
		}
		standardDeviationGray /= length;
		for (int i = 0; i < meanValue.rows; i++)
			for (int j = 0; j < meanValue.cols; j++)
			{
				standardDeviationGray.at<float>(i, j) = sqrt(standardDeviationGray.at<float>(i, j));
			}
		return standardDeviationGray;
	}
	else
	{
		for (int i = 0; i < meanValue.rows; i++)
		{
			for (int j = 0; j < meanValue.cols; j++)
			{
				for (int k = 0; k < channels; k++)
				{
					for (int l = 0; l < length; l++)
					{
						/*std::cout << standardDeviationColor.at<Vec3f>(i, j)[k] << "    ";
						std::cout << "MV " << meanValue.at<Vec3f>(i, j)[k] << */
						standardDeviationColor.at<Vec3f>(i, j)[k] += (meanValue.at<Vec3f>(i, j)[k] - frame[l].at<Vec3f>(i, j)[k]) *
							(meanValue.at<Vec3f>(i, j)[k] - frame[l].at<Vec3f>(i, j)[k]);
						/*std::cout << standardDeviationColor.at<Vec3f>(i, j)[k] << std::endl;*/
					}
				}
			}
		}
		standardDeviationColor /= length;
		for (int i = 0; i < meanValue.rows; i++)
			for (int j = 0; j < meanValue.cols; j++)
				for (int k = 0; k < meanValue.channels(); k++)
				{
					standardDeviationColor.at<Vec3f>(i, j)[k] = sqrt(standardDeviationColor.at<Vec3f>(i, j)[k]);
				}
		return standardDeviationColor;
	}
}

Mat detectMotionGaussian(Mat& currentFrame, Mat& meanValue, Mat& standardDeviation, int threshold, char  colorSpace)
{
	Mat  frameFloat;
	double a = 0.001;
	int difference = 0, channels, modify = 0;

	if ((colorSpace == 'g') || (colorSpace == 'G'))
	{
		channels = 1;
		cvtColor(currentFrame, currentFrame, CV_BGR2GRAY);
		currentFrame.convertTo(frameFloat, CV_32FC1);
	}
	else if ((colorSpace == 'h') || (colorSpace == 'H'))
	{
		channels = 2;
		cvtColor(currentFrame, currentFrame, CV_BGR2HSV);
		currentFrame.convertTo(frameFloat, CV_32FC3);
	}
	else if ((colorSpace == 'y') || (colorSpace == 'Y'))
	{
		channels = 3;
		cvtColor(currentFrame, currentFrame, CV_BGR2YCrCb);
		currentFrame.convertTo(frameFloat, CV_32FC3);
	}
	else if ((colorSpace == 'r') || (colorSpace == 'R'))
	{
		channels = 3;
		currentFrame.convertTo(frameFloat, CV_32FC3);
	}

	Mat motionMap(currentFrame.rows, currentFrame.cols, CV_8UC1); /// The difference image is initialized with the same amount of rows and columns as the function's frames.

	for (int i = 0; i < currentFrame.rows; i++)
	{
		for (int j = 0; j < currentFrame.cols; j++)
		{
			for (int k = 0; k < channels; k++)
			{
				if ( (channels == 3) || (channels == 2) )
				{
					modify += (abs(frameFloat.at<Vec3f>(i, j)[k] - meanValue.at<Vec3f>(i, j)[k]) > threshold * standardDeviation.at<Vec3f>(i, j)[k]);
				}
				else
				{
					modify = (abs(frameFloat.at<float>(i, j) - meanValue.at<float>(i, j)) > threshold * standardDeviation.at<float>(i, j));
				}

			}
			/*if ( ((channels == 3) && (modify>0)) || 
				((channels == 2) && (modify>0)) ||
				((channels == 1) && (modify==1)) )*/
			if (modify>0)
				motionMap.at<uchar>(i, j) = 255;
			else
			{
				motionMap.at<uchar>(i, j) = 0;
				//meanValue.at<Vec3f>(i, j)[0] = 0.999
			}
			modify = 0;
		}
	}
	return motionMap;
}

Mat detectMotion(Mat& frame1, Mat& frame2, int threshold, char update)//, char colorSpace)
{
	double learningRate = 0.01, foregroundLearningRate = 0.01;
	int rows = frame1.rows;
	int cols = frame2.cols;
	int channel[3];
	int difference = 0, channels = frame1.channels();

	Mat motionMap(rows, cols, CV_8UC1); /// The difference image is initialized with the same amount of rows and columns as the function's frames.
	/// 
	//if ((colorSpace == 'g') || (colorSpace == 'G'))
	//{
	//	cvtColor(frame1, frame1Trans, CV_BGR2GRAY); /// Both previous and current frames are converted to the Grayscale color space so that the
	//	/// computation of differential image is faster.
	//	cvtColor(frame2, frame2Trans, CV_BGR2GRAY);// Should it be RGB or BGR???
	//	channels = 1;
	//}
	//else if ((colorSpace == 'h') || (colorSpace == 'H'))
	//{
	//	cvtColor(frame1, frame1Trans, CV_BGR2HSV); /// Both previous and current frames are converted to the Grayscale color space so that the
	//	/// computation of differential image is faster.
	//	cvtColor(frame2, frame2Trans, CV_BGR2HSV);// Should it be RGB or BGR???
	//	channels = 2;
	//}
	//else if ((colorSpace == 'r') || (colorSpace == 'R'))
	//{
		/*frame1Trans = frame1.clone();
		frame2Trans = frame2.clone();*/
	//	channels = 3;
	/*}*/

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			for (int k = 0; k < channels; k++)
			{
				/*difference += (frame2Trans.at<Vec3b>(i, j)[k] - frame1Trans.at<Vec3b>(i, j)[k]) *
					(frame2Trans.at<Vec3b>(i, j)[k] - frame1Trans.at<Vec3b>(i, j)[k]);*/
				if (channels >1)
					difference += abs(frame2.at<Vec3b>(i, j)[k] - frame1.at<Vec3b>(i, j)[k]);
				else 
					difference += abs(frame2.at<uchar>(i, j) - frame1.at<uchar>(i, j));
			}
			

			/*difference/channels;*/
			/*sqrt(difference)*/
			if (difference / channels> threshold)
			{
				motionMap.at<uchar>(i, j) = 255;
				if ((update == 'y') || (update == 'Y'))
				{
					frame1.at<Vec3b>(i, j)[0] = (1 - foregroundLearningRate*learningRate)*frame1.at<Vec3b>(i, j)[0] + foregroundLearningRate*learningRate*frame2.at<Vec3b>(i, j)[0];
					frame1.at<Vec3b>(i, j)[1] = (1 - foregroundLearningRate*learningRate)*frame1.at<Vec3b>(i, j)[1] + foregroundLearningRate*learningRate*frame2.at<Vec3b>(i, j)[1];
					frame1.at<Vec3b>(i, j)[2] = (1 - foregroundLearningRate*learningRate)*frame1.at<Vec3b>(i, j)[2] + foregroundLearningRate*learningRate*frame2.at<Vec3b>(i, j)[2];
				}
				
			}
			else
			{
motionMap.at<uchar>(i, j) = 0;
if ((update == 'y') || (update == 'Y'))
{
	frame1.at<Vec3b>(i, j)[0] = (1 - learningRate)*frame1.at<Vec3b>(i, j)[0] + learningRate*frame2.at<Vec3b>(i, j)[0];
	frame1.at<Vec3b>(i, j)[1] = (1 - learningRate)*frame1.at<Vec3b>(i, j)[1] + learningRate*frame2.at<Vec3b>(i, j)[1];
	frame1.at<Vec3b>(i, j)[2] = (1 - learningRate)*frame1.at<Vec3b>(i, j)[2] + learningRate*frame2.at<Vec3b>(i, j)[2];
}

			}

			difference = 0;
		}
	}
	return motionMap;
}

void Morphology_Operations(int, void*)
{

	// Since MORPH_X : 2,3,4,5 and 6
	int operation = morph_operator + 2;

	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));

	/// Apply the specified morphology operation
	morphologyEx(motionMap, motionMapMorph, operation, element);
}

glm::vec2 handSimpleCrosshairControl(Mat& binaryMap, Mat& currentFrame, Mat& crosshairMap)
{
	Mat currentFrameHSV;
	cvtColor(currentFrame, currentFrameHSV, CV_BGR2HSV);
	int hue, saturation, value;
	int sensitivity = 2;
	bool changeMinRow = 0, changeMaxRow = 0, changeMinCol = 0, changeMaxCol = 0;
	glm::vec2 minRow = glm::vec2(binaryMap.rows, 0), maxRow = glm::vec2(0, 0),
		minCol = glm::vec2(0, binaryMap.cols), maxCol = glm::vec2(0, 0);
	int meanRow = 0, meanCol = 0, prevMeanRow, prevMeanCol, pixelCounter = 0;


	for (int i = 0; i < binaryMap.rows; i++)
		for (int j = 0; j < binaryMap.cols; j++)
		{
 		hue = 2 * currentFrameHSV.at<Vec3b>(i, j)[0];
		saturation = currentFrameHSV.at<Vec3b>(i, j)[1] * 100 / 255.0;
		value = currentFrameHSV.at<Vec3b>(i, j)[2] * 100 / 255.0;
		//if ((binaryMap.at<uchar>(i, j) == 255) && (value > 90))
		//if ((binaryMap.at<uchar>(i, j) == 255) && ((hue > 230) && (hue < 280) && (saturation > 60) && (value >60)))//Red
		if ((binaryMap.at<uchar>(i, j) == 255) && ((hue > 45) && (hue < 70) ) && (saturation > 70) && (value >50))//Yellow
		{
			crosshairMap.at<Vec3b>(i, j)[0] = 0;
			crosshairMap.at<Vec3b>(i, j)[1] = 246;
			crosshairMap.at<Vec3b>(i, j)[2] = 255;

			meanRow += i;
			meanCol += j;
			pixelCounter++;
		}
		else if ((i != meanCol) && (j != meanCol))
		{
			crosshairMap.at<Vec3b>(i, j)[0] = 0;
			crosshairMap.at<Vec3b>(i, j)[1] = 0;
			crosshairMap.at<Vec3b>(i, j)[2] = 0;
		}

		}

	if (pixelCounter > 0)
	{
		meanRow = meanRow*resizeScale / pixelCounter;
		meanCol = meanCol*resizeScale / pixelCounter;
		if ((abs(meanRow - currentCrosshairMeanRow) > sensitivity) && (abs(meanCol - currentCrosshairMeanCol) > sensitivity))
		{
			currentCrosshairMeanRow = meanRow;
			currentCrosshairMeanCol = meanCol;
			crosshairMap.at<Vec3b>(currentCrosshairMeanRow / resizeScale, currentCrosshairMeanCol / resizeScale)[0] = 0;
			crosshairMap.at<Vec3b>(currentCrosshairMeanRow / resizeScale, currentCrosshairMeanCol / resizeScale)[1] = 0;
			crosshairMap.at<Vec3b>(currentCrosshairMeanRow / resizeScale, currentCrosshairMeanCol / resizeScale)[2] = 255;
		}
		
		return glm::vec2(currentCrosshairMeanCol-25, currentCrosshairMeanRow-25-100);
	}
	else
		return glm::vec2(currentCrosshairMeanCol - 25, currentCrosshairMeanRow - 25 - 100);
	
}

void handSimpleTriggerControl(Mat& binaryMap, Mat& currentFrame, Mat& triggerMap, glm::vec2& greenMean, glm::vec2& redMean)
{
	Mat currentFrameHSV;
	cvtColor(currentFrame, currentFrameHSV, CV_BGR2HSV);
	int hue, saturation, value;
	int sensitivity = 2;
	bool changeMinRow = 0, changeMaxRow = 0, changeMinCol = 0, changeMaxCol = 0;
	glm::vec2 minRow = glm::vec2(binaryMap.rows, 0), maxRow = glm::vec2(0, 0),
		minCol = glm::vec2(0, binaryMap.cols), maxCol = glm::vec2(0, 0);
	int meanRowRed = 0, meanColRed = 0, pixelCounterRed = 0;
	int meanRowGreen = 0, meanColGreen = 0, pixelCounterGreen = 0;


	for (int i = 0; i < binaryMap.rows; i++)
		for (int j = 0; j < binaryMap.cols; j++)
		{
			hue = 2 * currentFrameHSV.at<Vec3b>(i, j)[0];
			saturation = currentFrameHSV.at<Vec3b>(i, j)[1] * 100 / 255.0;
			value = currentFrameHSV.at<Vec3b>(i, j)[2] * 100 / 255.0;
			//RED
			if ( (binaryMap.at<uchar>(i, j) == 255) && 
				(((hue > 345) && (hue <= 360)) || ((hue >= 0) && (hue < 15))) 
				&& ((saturation > 65) && (value > 50)) )//Red

			//if ((binaryMap.at<uchar>(i, j) == 255) && ((hue > 10) && (hue < 45) && (saturation > 40) && (value >40))) //Orange
			{
				triggerMap.at<Vec3b>(i, j)[0] = 0;
				triggerMap.at<Vec3b>(i, j)[1] = 0;
				triggerMap.at<Vec3b>(i, j)[2] = 255;
				meanRowRed += i;
				meanColRed += j;
				pixelCounterRed++;
			}
			//GREEN
			else if ((binaryMap.at<uchar>(i, j) == 255) && ((hue > 85) && (hue < 130) && (saturation > 55) && (value >50)))
			{
				triggerMap.at<Vec3b>(i, j)[0] = 0;
				triggerMap.at<Vec3b>(i, j)[1] = 255;
				triggerMap.at<Vec3b>(i, j)[2] = 0;

				meanRowGreen += i;
				meanColGreen += j;
				pixelCounterGreen++;
			}
			else if ((i != meanColGreen) && (j != meanColGreen))
			{
				triggerMap.at<Vec3b>(i, j)[0] = 0;
				triggerMap.at<Vec3b>(i, j)[1] = 0;
				triggerMap.at<Vec3b>(i, j)[2] = 0;
			}

		}

	if (pixelCounterGreen > 0)
	{
		meanRowGreen = meanRowGreen*resizeScale / pixelCounterGreen;
		meanColGreen = meanColGreen*resizeScale / pixelCounterGreen;
		if ((abs(meanRowGreen - currentTriggerMeanRowGreen) > sensitivity) && (abs(meanColGreen - currentTriggerMeanColGreen) > sensitivity))
		{
			currentTriggerMeanRowGreen = meanRowGreen;
			currentTriggerMeanColGreen = meanColGreen;
			triggerMap.at<Vec3b>(currentTriggerMeanRowGreen / resizeScale, currentTriggerMeanColGreen / resizeScale)[0] = 0;
			triggerMap.at<Vec3b>(currentTriggerMeanRowGreen / resizeScale, currentTriggerMeanColGreen / resizeScale)[1] = 0;
			triggerMap.at<Vec3b>(currentTriggerMeanRowGreen / resizeScale, currentTriggerMeanColGreen / resizeScale)[2] = 255;
		}

		greenMean = glm::vec2(currentTriggerMeanColGreen, currentTriggerMeanRowGreen);
	}
	else
		greenMean = glm::vec2(currentTriggerMeanColGreen, currentTriggerMeanRowGreen);


	if (pixelCounterRed > 0)
	{
		meanRowRed = meanRowRed*resizeScale / pixelCounterRed;
		meanColRed = meanColRed*resizeScale / pixelCounterRed;
		if ((abs(meanRowRed - currentTriggerMeanRowRed) > sensitivity) && (abs(meanColRed - currentTriggerMeanColRed) > sensitivity))
		{
			currentTriggerMeanRowRed = meanRowRed;
			currentTriggerMeanColRed = meanColRed;
			triggerMap.at<Vec3b>(currentTriggerMeanRowRed / resizeScale, currentTriggerMeanColRed / resizeScale)[0] = 0;
			triggerMap.at<Vec3b>(currentTriggerMeanRowRed / resizeScale, currentTriggerMeanColRed / resizeScale)[1] = 255;
			triggerMap.at<Vec3b>(currentTriggerMeanRowRed / resizeScale, currentTriggerMeanColRed / resizeScale)[2] = 0;
		}

		redMean = glm::vec2(currentTriggerMeanColRed, currentTriggerMeanRowRed);
	}
	else
		redMean = glm::vec2(currentTriggerMeanColRed, currentTriggerMeanRowRed);

}

// End of OpenCV part #4


#define CALL_LabelComponent(x,y,returnLabel) { STACK[SP] = x; STACK[SP+1] = y; STACK[SP+2] = returnLabel; SP += 3; goto START; }
#define RETURN { SP -= 3;                \
                 switch (STACK[SP+2])    \
				                  {                       \
                 case 1 : goto RETURN1;  \
                 case 2 : goto RETURN2;  \
                 case 3 : goto RETURN3;  \
                 case 4 : goto RETURN4;  \
                 default: return;        \
				                  }                       \
			                  }
#define X (STACK[SP-3])
#define Y (STACK[SP-2])

void LabelComponent(unsigned short* STACK, unsigned short width, unsigned short height, 
	unsigned char* input, int* output, int labelNo, unsigned short x, unsigned short y)
{
		STACK[0] = x;
		STACK[1] = y;
		STACK[2] = 0;  /* return - component is labelled */
		int SP = 3;
		int index;

	START: /* Recursive routine starts here */

		index = X + width*Y;
		if (input[index] == 0) RETURN;   /* This pixel is not part of a component */
		if (output[index] != 0) RETURN;   /* This pixel has already been labelled  */
		output[index] = labelNo;

		if (X > 0) CALL_LabelComponent(X - 1, Y, 1);   /* left  pixel */
	RETURN1:

		if (X < width - 1) CALL_LabelComponent(X + 1, Y, 2);   /* rigth pixel */
	RETURN2:

		if (Y > 0) CALL_LabelComponent(X, Y - 1, 3);   /* upper pixel */
	RETURN3:

		if (Y < height - 1) CALL_LabelComponent(X, Y + 1, 4);   /* lower pixel */
	RETURN4:

		RETURN;
	}


void LabelImage(unsigned short width, unsigned short height, unsigned char* input, int* output)
	{
		unsigned short* STACK = (unsigned short*)malloc(3 * sizeof(unsigned short)*(width*height + 1));

		int labelNo = 0;
		int index = -1;
		for (unsigned short y = 0; y < height; y++)
		{
			for (unsigned short x = 0; x < width; x++)
			{
				index++;
				if (input[index] == 0) continue;   /* This pixel is not part of a component */
				if (output[index] != 0) continue;   /* This pixel has already been labelled  */
				/* New component found */
				labelNo++;
				LabelComponent(STACK, width, height, input, output, labelNo, x, y);
			}
		}

		free(STACK);
	}