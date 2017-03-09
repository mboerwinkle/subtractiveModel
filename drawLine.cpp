#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "voxtree.h"
//some weird bastard of a line drawing algo. Almost bresenham but with z spliced in in a strange and terrible way.
extern Voxtree volume;
void drawPt(int x, int y, int z){
	x+=volume.size/2;//FIXME should be stored somewhere.
	y+=volume.size/2;
	z+=volume.size/2;//so that build volume centers on origin.
	volume.rm(x, y, z);
}
void drawLine(int x1, int y1, int z1, int x2, int y2, int z2){
	int rotated = 0;
	if(abs(x2-x1) < abs(y2-y1)){//rotate plane
		int temp = x1;
		x1 = -y1;
		y1 = temp;
		temp = x2;
		x2 = -y2;
		y2 = temp;
		rotated = 1;
	}
	if(x1 > x2){//line is going backwards; switch points
		int temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
		temp = z1;
		z1 = z2;
		z2 = temp;
	}
	double slope = (double)(y2-y1)/(double)(x2-x1);
	int sign = 1;
	if(slope < 0){
		sign = -1;
	}
	double err = 0;
	int fullXdist = x2-x1;
   	for(;x1 <= x2; x1++){
		double completion = 1.0-fabs((double)(x2-x1)/(double)(fullXdist));
		if(rotated){
			drawPt(y1, -x1, z1+completion*(z2-z1));
		}else{
			drawPt(x1, y1, z1+completion*(z2-z1));
		}
		err+=slope;
		if(fabs(err)>=1){
			y1+=sign;
			err-=sign;
		}
	}
}
