#include <stdio.h>
#include <stdlib.h>
#include "camera.h"
#include "voxtree.h"
extern Camera cam;
extern double distToCenter;
extern double volumeSideLen;
void frameProcess(Voxtree* volume, char* camview, double angle, double corrDistToCenter, double corrHorizOffset, double corrAxisX, double corrAxisY){
	/*
	* Coord = camera coordinates as far as the volume is concerned.
	* size = size of the volume (e.g. 512)
	* sidelen = user provided side length of the volume
	* dist = user provided distance from the center
	* coord/size = dist/sidelen
	* coord = dist * size / sidelen = "distToCenter*volume->size/volumeSideLen"
	*/
	char* view = (char*)calloc(cam.width*cam.height, sizeof(char));
	memcpy(view, camview, cam.width*cam.height*sizeof(char));
	double camPosX = (cos(angle)* corrDistToCenter*distToCenter - sin(angle)*corrHorizOffset)*(double)(volume->size)/volumeSideLen;
	double camPosY = (sin(angle)* corrDistToCenter*distToCenter + cos(angle)*corrHorizOffset)*(double)(volume->size)/volumeSideLen;
	double camPosZ = 0;
	//printf("Camera Coordinates: %.3lf %.3lf %.3lf\n", camPosX, camPosY, camPosZ);
	for(int x = 0; x < cam.width; x++){
		for(int y = 0; y < cam.height; y++){
			if(view[x+y*cam.width]){//cut out this vector
				//Try to get a pyramid with more than one pixel
				int diag = 0;//The number of pixels to increase the square by
				while(1){
					int bty = y+diag+1;//far corner of our next expansion
					int btx = x+diag+1;
					if(bty >= cam.height || btx >= cam.width) break; //useless to extend square beyond viewport
					int bad = 0;
					for(int tc = 0; tc <= diag+1; tc++){//test coordinate
						if(!view[(x+tc)+bty*cam.width] || !view[btx+(y+tc)*cam.width]){
							bad = 1;
							break;// don't extend the square into stuff that shouldn't be cut
						}
					}
					if(bad) break;//this proposed extension extended too far
					diag++;
				}
				//printf("found a region %d pixels square\n", diag);
				for(int bx = x; bx <= x+diag; bx++){//blank out the square so we don't redo it.
					for(int by = y; by <= y+diag; by++){
						view[bx+by*cam.width] = 0;
					}
				}
				//
				double v1[3], v2[3], v3[3], v4[3];
				double *v[4] = {v1, v2, v3, v4};
				cam.getVec(angle, (double)(x+diag+1), (double)(y+diag+1), v1, corrAxisX, corrAxisY);//vectors are from the near corner of x, y to the far corner of x+diag, y+diag.
				cam.getVec(angle, (double)(x+diag+1), (double)y, v2, corrAxisX, corrAxisY);
				cam.getVec(angle, (double)x, (double)y, v3, corrAxisX, corrAxisY);
				cam.getVec(angle, (double)x, (double)(y+diag+1), v4, corrAxisX, corrAxisY);
				volume->deletePyramidIntersections(camPosX+volume->size/2, camPosY+volume->size/2, camPosZ+volume->size/2, v);//the +volume->size/2 is because these voxels are counted from their corners.
			}
		}
		//printf("Done with X %d\n", x);
	}
	free(view);
}
extern double checkFrameQuality(Voxtree *volume, char* view, double angle, double corrDistToCenter, double corrHorizOffset, double corrAxisX, double corrAxisY){
	double camPosX = (cos(angle)* corrDistToCenter*distToCenter - sin(angle)*corrHorizOffset)*(double)(volume->size)/volumeSideLen;
	double camPosY = (sin(angle)* corrDistToCenter*distToCenter + cos(angle)*corrHorizOffset)*(double)(volume->size)/volumeSideLen;
	double camPosZ = 0;
	int targetPixCount = 0;//This is how many pixels we want to intersect a block.
	int fulfilledPixCount = 0;//This is how many pixels actually intersect a block.
	for(int x = 0; x < cam.width; x++){
		for(int y = 0; y < cam.height /*&& y < 315*/; y++){
			if(!view[x+y*cam.width]){//this vector should not have gotten cut
				double v1[3], v2[3], v3[3], v4[3];
				double *v[4] = {v1, v2, v3, v4};
				cam.getVec(angle, (double)(x+1), (double)(y+1), v1, corrAxisX, corrAxisY);//vectors are from the near corner of x, y to the far corner of x, y
				cam.getVec(angle, (double)(x+1), (double)y, v2, corrAxisX, corrAxisY);
				cam.getVec(angle, (double)x, (double)y, v3, corrAxisX, corrAxisY);
				cam.getVec(angle, (double)x, (double)(y+1), v4, corrAxisX, corrAxisY);
				int intersects = volume->doesPyramidIntersectFull(camPosX+volume->size/2, camPosY+volume->size/2, camPosZ+volume->size/2, v);//the +volume->size/2 is because these voxels are counted from their corners.
				if(intersects != -1){//-1 means this ray doesn't even intersect with the biggest level of volume
					targetPixCount++;
					if(intersects == 1){
						fulfilledPixCount++;
					}
				}
			}
		}
	}
	return ((double)fulfilledPixCount)/targetPixCount;
}
