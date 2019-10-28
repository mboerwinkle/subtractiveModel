#include <stdio.h>
#include <stdlib.h>
#include "camera.h"
#include "voxtree.h"
extern Camera cam;
extern Voxtree *volume;
extern double distToCenter;
extern double volumeSideLen;
extern int ProgressX;
void frameProcess(char* view, double angle){
	/*
	 * Coord = camera coordinates as far as the volume is concerned.
	 * size = size of the volume (e.g. 512)
	 * sidelen = user provided side length of the volume
	 * dist = user provided distance from the center
	 * coord/size = dist/sidelen
	 * coord = dist * size / sidelen = "distToCenter*volume->size/volumeSideLen"
	 */
	double camPosX = cos(angle)*  distToCenter*volume->size/volumeSideLen;
	double camPosY = sin(angle)*  distToCenter*volume->size/volumeSideLen;
	double camPosZ = 0;
	printf("Camera Coordinates: %.3lf %.3lf %.3lf\n", camPosX, camPosY, camPosZ);
	for(int x = 0; x < cam.width; x++){
		ProgressX = x;
		for(int y = 0; y < cam.height; y++){
			if(view[x+y*cam.width]){//cut out this vector
				double v1[3], v2[3], v3[3], v4[3];
				double *v[4] = {v1, v2, v3, v4};
				cam.getVec(angle, (double)x+0.5, (double)y+0.5, v1);
				cam.getVec(angle, (double)x+0.5, (double)y-0.5, v2);
				cam.getVec(angle, (double)x-0.5, (double)y-0.5, v3);
				cam.getVec(angle, (double)x-0.5, (double)y+0.5, v4);
				volume->deletePyramidIntersections(camPosX+volume->size/2, camPosY+volume->size/2, camPosZ+volume->size/2, v);
			}
		}
		//printf("Done with X %d\n", x);
	}
	free(view);
}
