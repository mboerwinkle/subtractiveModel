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
int distToCenter = 550*1.25;
int frames = 8;
Voxtree volume(160*1.25);
int rayDist;//absolute longest a ray has to travel.
Camera cam(1);

void* frameCapture(void *null);
void norm(double* vec);

extern void makeStl();
void* frameProcess(void *null);

int main(){
	rayDist = sqrt(3*volume.size*volume.size)*0.5+distToCenter;

	startWindowThread();
	pthread_t frameCapThread;
	pthread_create(&frameCapThread, NULL, frameCapture, NULL);
	int periodic = 0;
	while(true){
		delay(FRAMERATE);
		periodic++;
		if(periodic == FRAMERATE){
			periodic = 0;
			resizeWindow(cam.winName, cam.width, cam.height);
		}

		cam.grabFrame();
		while(cam.dataThreadLock){}
		cam.dataThreadLock = 1;
		cam.processFrame();
		GaussianBlur(cam.data, cam.data, Size(11,11), 0, 0);
//		medianBlur(cam.data, cam.data, 7);
		cam.dataThreadLock = 0;
		cam.showDark();
		cam.drawCross(cam.width/2, cam.height/2, 255, 0, 255);
		imshow(cam.winName, cam.drawData);
	}
}

void* frameCapture(void *null){
	volume = (char*)calloc(MAXX*MAXY*MAXZ, sizeof(char));
	memset(volume, '+', MAXX*MAXY*MAXZ);
	pthread_t frameProcThread[frames];
	for(int frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		printf("Press enter when oriented to %.2lf degrees\n", angle*180/M_PI);
		getchar();
		pthread_create(&(frameProcThread[frameIdx]), NULL, frameProcess, &angle);
	}
	while(volumeDataLock){}
	puts("Writing to file...");
	FILE* fp = fopen("output.dat", "w");
	fwrite(volume, sizeof(char), MAXX*MAXY*MAXZ, fp);
	fclose(fp);
	puts("Written to file.");
	puts("Creating STL.");
	makeStl(volume);
	puts("STL Created.");
	free(volume);
	return NULL;
}
void norm(double* vec){
        double dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
        vec[0]/=dist;
        vec[1]/=dist;
        vec[2]/=dist;
}
      
