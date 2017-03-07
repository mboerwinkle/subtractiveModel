#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "camera.h"
#include "delay.h"
using namespace cv;
#define FRAMERATE 60

double FOV = 0.976;
double VERT_FOV = 0.523;
int distToCenter = 550;
double interval = M_PI/8;
int MAXX = 160, MAXY = 160, MAXZ = 160;//octree
Camera cam(1);

void* frameCapture(void *null);
void norm(double* vec);
extern void drawLine(char* volume, int x1, int y1, int z1, int x2, int y2, int z2);
extern void stitchVoxels(char* volume);
extern void hollow(char* volume);
extern void makeStl(char* volume);

int main(){
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
	char *view = (char*)calloc(cam.width*cam.height, sizeof(char));
	char *volume = (char*)calloc(MAXX*MAXY*MAXZ, sizeof(char));
	memset(volume, '+', MAXX*MAXY*MAXZ);
	for(double angle = 0.0; angle < 2*M_PI; angle+=interval){
		//Defining camera location and direction camera is facing in rotated space. We keep the model space still (it doesnt rotate).
		int camPosX = cos(angle)*distToCenter;
		int camPosY = sin(angle)*distToCenter;
		int camPosZ = 0;
		double camDir[3] = {-(double)camPosX, -(double)camPosY, 0.0};
		norm(camDir);
////

		printf("Press enter when oriented to %.2lf degrees\n", angle*180/M_PI);
		getchar();
		while(cam.dataThreadLock){}
		cam.dataThreadLock = 1;
		for(int x = 0; x < cam.width; x++){
			for(int y = 0; y < cam.height; y++){
				view[x+y*cam.width] = (cam.getBrightness(x, y) < cam.darkThreshold)? 0 : 1;//if below threshold then save 0, else 1
			}
		}
		cam.dataThreadLock = 0;
		puts("Processing Frame...");
		int rayDist = sqrt(MAXX*MAXX+MAXY*MAXY+MAXZ*MAXZ)*0.5+distToCenter;//absolute longest a ray has to travel.
		double xsteps = sin(0.5*(FOV/cam.width))*2*rayDist;
		double ysteps = sin(0.5*(VERT_FOV/cam.height))*2*rayDist;
		printf("xsteps %lf ysteps %lf\n", xsteps, ysteps);
		for(int x = 0; x < cam.width; x++){
			for(int y = 0; y < cam.height; y++){
				char val = view[x+y*cam.width];
				if(val){//cut out this vector//FIXME line drawing robust to big deltaZ//FIXME dynamic stl file size.
					for(double dlocx = -0.5; dlocx < 0.5; dlocx+=1/xsteps){
					for(double dlocy = -0.5; dlocy < 0.5; dlocy+=1/ysteps){
						double loc[2] = {((double)x+dlocx)/cam.width-0.5, ((double)y+dlocy)/cam.height-0.5};//location of the point on the view screen.
						double dirAngle = loc[0]*FOV;//angle from camera forwards
						double dirPitch = loc[1]*VERT_FOV;//angle upwards from camera forwards
						double finalAngle = angle+M_PI+dirAngle;//absolute angle
						double vec[3] = {cos(finalAngle)*cos(dirPitch),sin(finalAngle)*cos(dirPitch),sin(dirPitch)};//absolute point direction vector
						drawLine(volume, camPosX, camPosY, camPosZ, camPosX+vec[0]*rayDist, camPosY+vec[1]*rayDist, camPosZ+vec[2]*rayDist);
					}}
				}
			}
		}
		puts("Processing Done.");
	}
	free(view);
	puts("Stitching Voxels...");
	stitchVoxels(volume);
	puts("Voxels Stitched.");
	puts("Hollowing Model.");
	hollow(volume);
	puts("Model Hollowed.");
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
      
