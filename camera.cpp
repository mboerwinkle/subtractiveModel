#include <stdio.h>
#include <stdlib.h>
#include "camera.h"


Camera::Camera(int idx){
	sprintf(winName, "Camera Feed %d", idx);
	namedWindow(winName, CV_WINDOW_AUTOSIZE);
	cam.open(idx);
	if(!cam.isOpened()){
		printf("Failed to open camera %d\n", idx);
	}
	grabFrame();
	processFrame();
	width = data.size().width;
	height = data.size().height;
}
void Camera::showDark(){
	for(int x = 0; x < width; x++){
		for(int y = 0; y < height; y++){
			if(getBrightness(x, y) < darkThreshold){
				drawData.at<cv::Vec3b>(y, x)[0] = 75;
				drawData.at<cv::Vec3b>(y, x)[1] = 50;
				drawData.at<cv::Vec3b>(y, x)[2] = 200;
				
			}
		}
	}
}
void Camera::drawCross(int x, int y, int r, int g, int b){
	for(int tempx = x-3; tempx <=x+3; tempx++){
	for(int tempy = y-3; tempy <=y+3; tempy++){
		drawData.at<cv::Vec3b>(tempy, tempx)[0] = b;//coloridx here
		drawData.at<cv::Vec3b>(tempy, tempx)[1] = g;
		drawData.at<cv::Vec3b>(tempy, tempx)[2] = r;
	}}
}
int Camera::getRed(int x, int y){
	return getComp(x, y, 'r');
}
int Camera::getGreen(int x, int y){
	return getComp(x, y, 'g');
}
int Camera::getBlue(int x, int y){
	return getComp(x, y, 'b');
}
int Camera::getComp(int x, int y, char c){
	int coloridx;//coloridx here
	if(c == 'r'){
		coloridx = 2;
	}else if(c == 'g'){
		coloridx = 1;
	}else if(c == 'b'){
		coloridx = 0;
	}
	return data.at<cv::Vec3b>(y, x)[coloridx];
}
int Camera::getBrightness(int x, int y){
	return (getRed(x, y)+getBlue(x, y)+getGreen(x, y))/3;
}

void Camera::processFrame(){
	cam.retrieve(data);
	cam.retrieve(drawData);
}
void Camera::grabFrame(){
	cam.grab();
}
