#ifndef CAMERA_H
#define CAMERA_H
#include <opencv2/opencv.hpp>
using namespace cv;
class Camera{
public:
	Camera(int idx);
	char winName[80];
	//writes camera output to internal frame storage.
	void processFrame();
	//Quickly grabs the next frame...
	void grabFrame();
	void showDark();
	int darkThreshold = 50;
	Mat data;
	int dataThreadLock = 0;
	Mat drawData;
	int width, height;
	void drawCross(int x, int y, int r, int g, int b);
	int getRed(int x, int y);
	int getBlue(int x, int y);
	int getGreen(int x, int y);
	int getComp(int x, int y, char c);
	int getBrightness(int x, int y);
private:
	cv::VideoCapture cam;
};
#endif
