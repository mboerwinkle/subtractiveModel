#include <stdio.h>
#include <stdlib.h>
#include "camera.h"
#include "voxtree.h"
extern Camera cam;
extern Voxtree volume;
extern double FOV, VERT_FOV;//FIXME make attribute of camera
extern int distToCenter;
extern int rayDist;
extern void drawLine(int x1, int y1, int z1, int x2, int y2, int z2);
void frameProcess(char* view, double angle){
	int camPosX = cos(angle)*distToCenter;
	int camPosY = sin(angle)*distToCenter;
	int camPosZ = 0;
	for(int x = 0; x < cam.width; x++){
		for(int y = 0; y < cam.height; y++){
			if(view[x+y*cam.width]){//cut out this vector//FIXME line drawing robust to big deltaZ//FIXME dynamic stl file size.
				double vec1[3], vec2[3];
				cam.getVec(angle, (double)x+0.5, (double)y+0.5, vec1);
				cam.getVec(angle, (double)x-0.5, (double)y-0.5, vec2);
			//	volume.deletePyramidIntersections(camPosX+volume.size/2, camPosY+volume.size/2, camPosZ+volume.size/2, vec1, vec2);
				volume.deleteLineIntersections(camPosX+volume.size/2, camPosY+volume.size/2, camPosZ+volume.size/2, vec1);
			}
		}
		printf("Done with X %d\n", x);
	}
	free(view);
}
