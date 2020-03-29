#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "camera.h"
#include "voxtree.h"
#include "delay.h"
using namespace cv;
extern int totalNodeCount;
extern int peakNodeCount;
#define FRAMERATE 20
double distToCenter = 3520/2;
double volumeSideLen = 0;
int frames = 0;//This is the number of frames we want to take.
int frameIdx = -1;//This is the frame currently being processed.
int ProgressX = 0;//This is used to give updates about the processing progress.

Voxtree *volume;

Camera cam;
bool stillCapturing = true;

void* frameCapture(void *null);

extern void makeStl();
void frameProcess(char* view, double angle);

int main(int argc, char *argv[]){
	if(argc != 6){
		printf("USAGE: ./subtractiveModel VIDEO_SRC_IDX FRAMES DIST_TO_CENTER VOLUME_SIDE_LENGTH MODEL_RESOLUTION (ex. \"./subtractiveModel 0 12 90.0 40.0 500\"\n");
		return 1;
	}

	cam.assignFeed(atoi(argv[1]));
	sscanf(argv[2], "%d", &frames);
	sscanf(argv[3], "%lf", &distToCenter);
	sscanf(argv[4], "%lf", &volumeSideLen);
	printf("Frames: %d  DistToCenter: %.3lf  VolumeSideLen: %.3lf\n", frames, distToCenter, volumeSideLen);
	volume = new Voxtree(atof(argv[5]));
	printf("Each voxel is %.3lf^3\n", volumeSideLen/volume->size);
	//printf("Each pixel is approximately %.3lf voxels. Below ~1 voxel you are wasting memory and should use a lower model resolution. Above ~1 voxel you are wasting camera resolution.\n", 0.0);
	//startWindowThread();

	pthread_t frameCapThread;
	pthread_create(&frameCapThread, NULL, frameCapture, NULL);
	int periodic = 0;
	while(stillCapturing){
		//delay(FRAMERATE);
		periodic++;
		if(periodic == FRAMERATE){
			periodic = 0;
			//printf("resizing window. X:%d Y:%d\n", cam.width, cam.height);
			resizeWindow(cam.winName, cam.width, cam.height);
		}

		cam.grabFrame();
		cam.processFrame();//FIXME semaphores
//		GaussianBlur(cam.data, cam.data, Size(11,11), 0, 0);
		medianBlur(cam.data, cam.data, 5);
		cam.showDark();
		for(int x = 0; x < 15; x++){
			for(int y = 0; y < 5; y++){
				cam.drawCross(cam.width*(x+1)/(15+1), cam.height*(y+1)/(5+1), 255, 0, 255);
			}
		}
		imshow(cam.winName, cam.drawData);
		waitKey(1000.0/(double)(FRAMERATE));
	}
	cam.deleteFeed();

	printf("Frame gathering complete. Press enter to view completion.\n");
	while(1){
		getchar();
		printf("Frame %d: X Progress: %d/%d\n", frameIdx, ProgressX, cam.width);
	}
	//pthread_join(frameCapThread, NULL);
}

void* frameCapture(void *null){
	char *view[frames];
//	Mat dataCopy;
	for(frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		printf("Press enter when oriented to %.2lf degrees\n", angle*180/M_PI);
		getchar();
		printf("Copying Data for Frame %d/%d!\n", frameIdx+1, frames);
		//imwrite("out.png", cam.data);
//		cam.getData(dataCopy);
		view[frameIdx] = (char*)calloc(cam.width*cam.height, sizeof(char));
		sem_wait(&(cam.dataMutex));
		for(int x = 0; x < cam.width; x++){
			for(int y = 0; y < cam.height; y++){
				view[frameIdx][x+y*cam.width] = (cam.getBrightness(x, y) < cam.darkThreshold)? 0 : 1;//if below threshold then save 0, else 1
			}
		}
		sem_post(&(cam.dataMutex));

	}
	stillCapturing = false;
	for(frameIdx = 0; frameIdx < frames; frameIdx++){
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
      
