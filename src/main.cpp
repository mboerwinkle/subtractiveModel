#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "camera.h"
#include "voxtree.h"
#include "delay.h"
#include "hillClimb.h"
using namespace cv;
extern int totalNodeCount;
extern int peakNodeCount;
#define FRAMERATE 20
double distToCenter = 3520/2;
double volumeSideLen = 0;
int frames = 0;//This is the number of frames we want to take.
int frameIdx = -1;//This is the frame currently being processed.
int modelRes = 0;//This is based on a user-provided argument. It is used in the Voxtree constructor.
int loadFromImages = 0;//This is chosen to load from the auto-saved snapshots.
int maxIterations = 0;
Camera cam;
bool stillCapturing = true;

void* frameCapture(void *null);

extern void makeStl(Voxtree* volume);
extern void frameProcess(Voxtree *volume, char* camview, double angle, double corrDistToCenter, double corrHorizOffset, double corrAxisX, double corrAxisY);
extern double checkFrameQuality(Voxtree *volume, char* view, double angle, double corrDistToCenter, double corrHorizOffset, double corrAxisX, double corrAxisY);
Voxtree* createModel(char** view, double corrDistToCenter, double corrHorizOffset, double corrAxisX, double corrAxisY);
double getCorrectionScore(void* view_void, double* args);
int main(int argc, char *argv[]){
	if(argc != 7){
		printf("USAGE: ./subtractiveModel VIDEO_SRC_IDX FRAMES DIST_TO_CENTER VOLUME_SIDE_LENGTH MODEL_RESOLUTION MAX_CORRECTION_ITERATIONS (ex. \"./subtractiveModel 0 12 90.0 40.0 500 0\"\n");
		return 1;
	}

	sscanf(argv[2], "%d", &frames);
	sscanf(argv[3], "%lf", &distToCenter);
	sscanf(argv[4], "%lf", &volumeSideLen);
	printf("Frames: %d  DistToCenter: %.3lf  VolumeSideLen: %.3lf\n", frames, distToCenter, volumeSideLen);
	modelRes = Voxtree::equalOrGreaterPow2(atof(argv[5]));
	maxIterations = atoi(argv[6]);
	

	printf("Each voxel is %.3lf^3\n", volumeSideLen/modelRes);
	//printf("Each pixel is approximately %.3lf voxels. Below ~1 voxel you are wasting memory and should use a lower model resolution. Above ~1 voxel you are wasting camera resolution.\n", 0.0);
	//startWindowThread();
	int camIdx = atoi(argv[1]);
	if(camIdx == -1){
		loadFromImages = 1;
	}else{
		cam.assignFeed(camIdx);
	}
	pthread_t frameCapThread;
	pthread_create(&frameCapThread, NULL, frameCapture, NULL);
	if(!loadFromImages){
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
	}
	printf("Frame gathering complete.\n");
	pthread_join(frameCapThread, NULL);
}

void* frameCapture(void *null){
	char *view[frames];
	for(frameIdx = 0; frameIdx < frames; frameIdx++){
		char outname[80];//picture name for this image
		sprintf(outname, "out%d.png", frameIdx);
		if(loadFromImages){
			cam.data = imread(outname);
			if(cam.data.data == NULL){
				printf("Could not open file %s\n", outname);
				return NULL;
			}else{
				cam.width = cam.data.cols;
				cam.height = cam.data.rows;
			}
		}else{
			printf("Press enter when oriented to %.2lf degrees", frameIdx*360.0/frames);
			getchar();
			printf("Copying Data for Frame %d/%d!\n", frameIdx+1, frames);
			imwrite(outname, cam.data);
		}
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

	/*Possible inaccuracies to be corrected via hill-climbing:
	 * Inaccurate distToCenter (search idx 0)
	 * Camera Center vector does not intersect rotation axis (search idx 1)
	 * Rotation axis is not parallel to axis protruding from top of camera (search idx 2 and 3)
	 */
	
	double min[4]=	{0.9, -(distToCenter*0.01*((double)cam.width)/cam.NewCameraMatrix.at<double>(0)), -0.1745329, -0.1745329};
	//distToCenter multiplier (can be off by +-10%)
	//center vector can be off by +-1% of the camera width
	//rotation axis can be off by 10 degrees (That is, 10 degrees camera rotation around fixed distant point, or 10 degrees camera looking up or down. It is important to perform the looking up or down translation first.
	double max[4]=	{1.1, -min[1], -min[2], -min[3]};
	double step[4];
	for(int idx = 0; idx < 4; idx++){
		step[idx] = (max[idx]-min[idx])/20.0;
	}
	double* optimal = hillClimb(4, min, max, step, view, &getCorrectionScore, maxIterations);
	Voxtree *volume = createModel(view, optimal[0], optimal[1], optimal[2], optimal[3]);
	puts("Creating STL.");
	makeStl(volume);
	puts("STL Created.");
	delete volume;
	return NULL;
}
double getCorrectionScore(void* view_void, double* args){
	char** view = (char**) view_void;
	double corrDistToCenter = args[0];
	double corrHorizOffset = args[1];
	double corrAxisX = args[2];
	double corrAxisY = args[3];
	Voxtree* volume = createModel(view, corrDistToCenter, corrHorizOffset, corrAxisX, corrAxisY);
	double quality[frames];
	for(frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		quality[frameIdx] = checkFrameQuality(volume, view[frameIdx], angle, corrDistToCenter, corrHorizOffset, corrAxisX, corrAxisY);
	}
	for(int mergeIdx = 2; mergeIdx < frames*2; mergeIdx*=2){//This is a tree-based multiply together algorithm to avoid frame favoritism from imprecise multiplication (accumulates in quality[0])
		for(int idx = 0; idx < frames; idx+=mergeIdx){
			if(idx+mergeIdx/2 < frames){
				quality[idx] *= quality[idx+mergeIdx/2];
			}
		}
	}
	delete volume;
	return quality[0];
}

Voxtree* createModel(char** view, double corrDistToCenter, double corrHorizOffset, double corrAxisX, double corrAxisY){
	Voxtree *volume = new Voxtree(modelRes);
	for(frameIdx = 0; frameIdx < frames; frameIdx++){
		double angle = frameIdx*2*M_PI/frames;
		frameProcess(volume, view[frameIdx], angle, corrDistToCenter, corrHorizOffset, corrAxisX, corrAxisY);
	}
	return volume;
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
      
