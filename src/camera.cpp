#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include "camera.h"
#include "loadJsonConfig.h"

extern void norm(double* v);
//hardcoded camera calibration values.
double cmValues[9] = {541.2580031887313, 0,317.4572030174616, 0,541.6212403166173, 235.17797199583416,0,0,1};
double dcValues[5] = {-0.11403991045980194,0.20117497198196835,-0.0006543861951229738,0.0029130428627756684,-0.1081702720175864};
double newCmValues[9];

char configFileName[80] = "default.json";

Camera::Camera(){
	loadCameraParams(configFileName);
	sprintf(winName, "Camera Feed");
	namedWindow(winName, cv::WINDOW_AUTOSIZE);
	//create the camera Matrix
	CameraMatrix = Mat(3, 3, CV_64FC1, cmValues);
	//create the distortion coefficient matrix
	DistortionCoef = Mat(5, 1, CV_64FC1, dcValues);
	//create the new camera data
	memcpy(newCmValues, cmValues, 9*sizeof(double));
	NewCameraMatrix = Mat(3, 3, CV_64FC1, newCmValues);
	sem_init(&dataMutex, 0, 1);
}
void Camera::loadCameraParams(char* fname){
	FILE* fp = fopen(fname, "r");
	if(fp == NULL){
		printf("%s not found. Using defaults. (Highly not recommended)\n", configFileName);
		return;
	}
	jsonValue* calibData = jsonLoad(fp);
	fclose(fp);
	printf("distortion model: \"%s\"\n", jsonGetString(jsonGetObj(*calibData, "distortion_model")));
	jsonValue cm = jsonGetObj(*calibData, "camera_matrix");
	jsonValue r0 = jsonGetArr(cm, 0);
	jsonValue r1 = jsonGetArr(cm, 1);
	jsonValue r2 = jsonGetArr(cm, 2);
	for(int idx = 0; idx < 3; idx++){
		cmValues[idx] = jsonGetDouble(jsonGetArr(r0, idx));
		cmValues[idx+3] = jsonGetDouble(jsonGetArr(r1, idx));
		cmValues[idx+6] = jsonGetDouble(jsonGetArr(r2, idx));
	}
	jsonValue distortion = jsonGetObj(*calibData, "distortion_coefficients");
	if(5 != jsonGetLen(distortion)){
		printf("Unknown distortion. %d distortion coef\n", jsonGetLen(distortion));
	}
	for(int idx = 0; idx < jsonGetLen(distortion); idx++){
		dcValues[idx] = jsonGetDouble(jsonGetArr(distortion, idx));
	}
	jsonFree(*calibData);
	free(calibData);
}
void Camera::assignFeed(int idx){
	printf("Assigning Camera Feed %d\n", idx);
	char newName[80];
	sprintf(newName, "Camera Feed %d", idx);
	setWindowTitle(winName, newName);
	cam.open(idx);
	if(!cam.isOpened()){
		printf("Failed to open camera %d\n", idx);
		return;
	}
//	sem_init(&dataMutex, 0, 1);
	grabFrame();
	processFrame();
}
void Camera::deleteFeed(){
	destroyWindow(winName);
	cam.release();
	sem_destroy(&dataMutex);
}
/*void Camera::getData(Mat write){
	sem_wait(&dataMutex);
	data.copyTo(write);
	sem_post(&dataMutex);
}
*/
void Camera::showDark(){
	for(int x = 0; x < width; x++){
		for(int y = 0; y < height; y++){
			if(getBrightness(x, y) < darkThreshold){
				drawData.at<cv::Vec3b>(y, x)[0] = 0;
				drawData.at<cv::Vec3b>(y, x)[1] = 250;
				drawData.at<cv::Vec3b>(y, x)[2] = 0;
				
			}
		}
	}
}
void Camera::drawCross(int x, int y, int r, int g, int b){
	for(int tempx = x-2; tempx < x+2; tempx++){
	for(int tempy = y-2; tempy < y+2; tempy++){
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
/*
void Camera::getVec(double angle, double x, double y, double* out){
	double loc[2] = {x/width-0.5, y/height-0.5};//location of the point on the view screen. (0,0) is center. Range -0.5 to 0.5.
	double dirAngle = loc[0]*FOV;//angle from camera forwards
	double dirPitch = loc[1]*VERT_FOV;//angle upwards from camera forwards
	double finalAngle = angle+M_PI+dirAngle;//absolute angle
	out[0] = cos(finalAngle)*cos(dirPitch);
	out[1] = sin(finalAngle)*cos(dirPitch);
	out[2] = sin(dirPitch);//absolute point direction vector
//	printf("%lf\n", out[0]*out[0]+out[1]*out[1]+out[2]*out[2]);
}
*/
/*
 * Camera screen coordinates U, V. Real coordinates x,y,z.
 * fx focal length x (cmValues[0]), fy focal length y (cmValues[4]).
 * Cx, Cy principal points (center of camera screen x and y) cmValues[2] & [5].
 * 
 * U = fx * (x/z) + Cx
 * 	x/z = (U - Cx) / fx
 * V = fy * (y/z) + Cy
 * 	y/z = (V - Cy) / fy
*/

void Camera::getVec(double angle, double U, double V, double* out){
	
	//z is 1. "vec[0]" is really x/z, "vec[1]" is y/z.
	double vec[3];
	vec[0] = (double)(U - width/2) / newCmValues[0];
	vec[1] = (double)(V - height/2) / newCmValues[4];
	vec[2] = 1;
	//now vec is the vector with 'z' pointing straight out of the camera.
	
	norm((double*)vec);
	//now vec is a unit vector.
	
	double t = vec[2];
	vec[2] = vec[1];
	vec[1] = vec[0];
	vec[0] = t;
	//now vec shares the same coordinates as global, but rotated around vertical z.
	
	out[2] = vec[2];//vertical is unchanged
	double c = cos(angle + M_PI);
	double s = sin(angle + M_PI);
	out[0] = vec[0]*c - vec[1]*s;
	out[1] = vec[1]*c + vec[0]*s;
	//printf("%.3lf, %.3lf, %.3lf\n", out[0], out[1], out[2]);
	//printf("getVec Len: %lf\n", sqrt(out[0]*out[0]+out[1]*out[1]+out[2]*out[2]));
}

void Camera::processFrame(){
	sem_wait(&dataMutex);
	//put the frame in data
	cam.retrieve(data);
	//then undistort it into drawdata
	undistort(data, drawData, CameraMatrix, DistortionCoef, NewCameraMatrix);
	//then clone back into data. By this point drawData and data are the same.
	data = drawData.clone();
	sem_post(&dataMutex);
	//update our width and height knowledge
	int w = data.size().width;
	int h = data.size().height;
	if(w != width || h != height){
		printf("dimensions changed: W %d->%d H %d->%d\n", width, w, height, h);
	}
	width = w;
	height = h;
	//set the camera new principle point to the center of the screen/image.
	NewCameraMatrix.at<double>(2) = w/2;
	NewCameraMatrix.at<double>(5) = h/2;
}
void Camera::grabFrame(){
	cam.grab();
}
