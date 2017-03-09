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
int someoneInside = 0;
void frameProcess(char* view, double angle){
	int camPosX = cos(angle)*distToCenter;
	int camPosY = sin(angle)*distToCenter;
	int camPosZ = 0;
	double xsteps = sin(0.5*(FOV/cam.width))*2*rayDist;
	double ysteps = sin(0.5*(VERT_FOV/cam.height))*2*rayDist;
	printf("xsteps %lf ysteps %lf\n", xsteps, ysteps);
	for(int x = 0; x < cam.width; x++){
		for(int y = 0; y < cam.height; y++){
			if(view[x+y*cam.width]){//cut out this vector//FIXME line drawing robust to big deltaZ//FIXME dynamic stl file size.
				for(double dlocx = -0.5; dlocx < 0.5; dlocx+=1/xsteps){
					for(double dlocy = -0.5; dlocy < 0.5; dlocy+=1/ysteps){
						double loc[2] = {((double)x+dlocx)/cam.width-0.5, ((double)y+dlocy)/cam.height-0.5};//location of the point on the view screen.
						double dirAngle = loc[0]*FOV;//angle from camera forwards
						double dirPitch = loc[1]*VERT_FOV;//angle upwards from camera forwards
						double finalAngle = angle+M_PI+dirAngle;//absolute angle
						double vec[3] = {cos(finalAngle)*cos(dirPitch),sin(finalAngle)*cos(dirPitch),sin(dirPitch)};//absolute point direction vector
					//	drawLine(camPosX, camPosY, camPosZ, camPosX+vec[0]*rayDist, camPosY+vec[1]*rayDist, camPosZ+vec[2]*rayDist);
						volume.deleteLineIntersections(camPosX+volume.size/2, camPosY+volume.size/2, camPosZ+volume.size/2, vec);
					}
				}
			}
		}
		printf("Done with X %d\n", x);
	}
	free(view);
}
