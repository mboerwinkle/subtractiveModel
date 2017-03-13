#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "camera.h"
#include "voxtree.h"
#include "delay.h"
using namespace cv;
#define FRAMERATE 60

double FOV = 0.976;
double VERT_FOV = 0.523;
int distToCenter = 3520;
int frames = 4;
Voxtree volume(1024);
int rayDist;//absolute longest a ray has to travel.
Camera cam(1);
bool stillCapturing = true;

void* frameCapture(void *null);

extern void makeStl();
void frameProcess(char* view, double angle);

int main(){
	rayDist = sqrt(3*volume.size*volume.size)*0.5+distToCenter;

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
		GaussianBlur(cam.data, cam.data, Size(11,11), 0, 0);
//		medianBlur(cam.data, cam.data, 7);
		cam.showDark();
		cam.drawCross(cam.width/2, cam.height/2, 255, 0, 255);
		imshow(cam.winName, cam.drawData);
	}
	cam.deleteFeed();
	pthread_join(frameCapThread, NULL);
}

void* frameCapture(void *null){
	char *(view[frames]);
	for(int frameIdx = 0; frameIdx < frames; frameIdx++){

		view[frameIdx] = (char*)calloc(cam.width*cam.height, sizeof(char));
		for(int x = 0; x < cam.width; x++){
			for(int y = 0; y < cam.height; y++){
				view[frameIdx][x+y*cam.width] = (cam.getBrightness(x, y) < cam.darkThreshold)? 0 : 1;//if below threshold then save 0, else 1
			}
		}
		double angle = frameIdx*2*M_PI/frames;
		printf("Press enter when oriented to %.2lf degrees\n", angle*180/M_PI);
		getchar();
	}
	stillCapturing = false;
	for(int frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		frameProcess(view[frameIdx], angle);
		printf("Frame %d complete!\n", frameIdx);
		printf("DataSize is %d\nWould be %d oldstyle\n", volume.dataSize(), volume.size*volume.size*volume.size);
	}
	puts("Creating STL.");
	makeStl();
	puts("STL Created.");
	return NULL;
}
      
