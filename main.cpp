#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "camera.h"
#include "voxtree.h"
#include "delay.h"
using namespace cv;
extern int totalNodeCount;
extern int peakNodeCount;
#define FRAMERATE 60
int distToCenter = 3520/2;
int frames = 12;
int ProgressX = 0;
Voxtree volume(1024/2);
Camera cam(1);
bool stillCapturing = true;

void* frameCapture(void *null);

extern void makeStl();
void frameProcess(char* view, double angle);

int main(){
	startWindowThread();
	pthread_t frameCapThread;
	pthread_create(&frameCapThread, NULL, frameCapture, NULL);
	int periodic = 0;
	while(stillCapturing){
		delay(FRAMERATE);
		periodic++;
		if(periodic == FRAMERATE){
			periodic = 0;
			resizeWindow(cam.winName, cam.width, cam.height);
		}

		cam.grabFrame();
		cam.processFrame();//FIXME semaphores
//		GaussianBlur(cam.data, cam.data, Size(11,11), 0, 0);
		medianBlur(cam.data, cam.data, 5);
		cam.showDark();
		cam.drawCross(cam.width/2, cam.height/2, 255, 0, 255);
		imshow(cam.winName, cam.drawData);
	}
	cam.deleteFeed();
	while(true){
		getchar();
		printf("feed x coord:%d\n", ProgressX);
		
	}
//	pthread_join(frameCapThread, NULL);
}

void* frameCapture(void *null){
	char *(view[frames]);
//	Mat dataCopy;
	for(int frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		printf("Press enter when oriented to %.2lf degrees\n", angle*180/M_PI);
		getchar();
//		cam.getData(dataCopy);
		view[frameIdx] = (char*)calloc(cam.width*cam.height, sizeof(char));
		for(int x = 0; x < cam.width; x++){
			for(int y = 0; y < cam.height; y++){
				view[frameIdx][x+y*cam.width] = (cam.getBrightness(x, y) < cam.darkThreshold)? 0 : 1;//if below threshold then save 0, else 1
			}
		}

	}
	stillCapturing = false;
	for(int frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		frameProcess(view[frameIdx], angle);
		printf("Frame %d complete!\n", frameIdx);
		printf("Node Count is %d\n", totalNodeCount);
	}
	printf("Peak Node Count is %d\n", peakNodeCount);
	puts("Creating STL.");
	makeStl();
	puts("STL Created.");
	return NULL;
}
void norm(double* v){
	double d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	v[0]/=d;
	v[1]/=d;
	v[2]/=d;
}
void cross(double* tv, double* v1, double* v2){//FIXME aliases? inline them?
	tv[0] = v1[1]*v2[2]-v1[2]*v2[1];
	tv[1] = v1[2]*v2[0]-v1[0]*v2[2];
	tv[2] = v1[0]*v2[1]-v1[1]*v2[0];
}
double dot(double* v1, double* v2){
	return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}
      
