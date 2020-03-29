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
	double camPosX = cos(angle)*  distToCenter*(double)(volume->size)/volumeSideLen;
	double camPosY = sin(angle)*  distToCenter*(double)(volume->size)/volumeSideLen;
	double camPosZ = 0;
	printf("Camera Coordinates: %.3lf %.3lf %.3lf\n", camPosX, camPosY, camPosZ);
	for(int x = 0; x < cam.width; x++){
		ProgressX = x;
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
				cam.getVec(angle, (double)(x+diag+1), (double)(y+diag+1), v1);//vectors are from the near corner of x, y to the far corner of x+diag, y+diag.
				cam.getVec(angle, (double)(x+diag+1), (double)y, v2);
				cam.getVec(angle, (double)x, (double)y, v3);
				cam.getVec(angle, (double)x, (double)(y+diag+1), v4);
				volume->deletePyramidIntersections(camPosX+volume->size/2, camPosY+volume->size/2, camPosZ+volume->size/2, v);//the +volume->size/2 is because these voxels are counted from their corners.
			}
		}
		//printf("Done with X %d\n", x);
	}
	free(view);
}
