#ifndef CAMERA_H
#define CAMERA_H
#include <semaphore.h>
#include <opencv2/opencv.hpp>
using namespace cv;
class Camera{
public:
	Camera(int idx);
	void deleteFeed();
	char winName[80];
double FOV = 0.976;//FIXME delete
double VERT_FOV = 0.523;
	//writes camera output to internal frame storage.
	void processFrame();
	//Quickly grabs the next frame...
	void grabFrame();
	void showDark();
	int darkThreshold = 50;
	Mat drawData;
	Mat data;
	int width, height;
//	void getData(Mat write);
	void drawCross(int x, int y, int r, int g, int b);
	int getRed(int x, int y);
	int getBlue(int x, int y);
	int getGreen(int x, int y);
	int getComp(int x, int y, char c);
	int getBrightness(int x, int y);
	void getVec(double angle, double x, double y, double* out);
private:
//	sem_t dataMutex;
	cv::VideoCapture cam;
};
#endif
