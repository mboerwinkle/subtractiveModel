#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//some weird bastard of a line drawing algo. Almost bresenham but with z spliced in in a strange and terrible way.
extern int MAXX, MAXY, MAXZ;
extern int idx(int x, int y, int z);
void drawPt(char* volume, int x, int y, int z){
	x+=MAXX/2;
	y+=MAXY/2;
	z+=MAXZ/2;//so that build volume centers on origin.
	if(x >=0 && x < MAXX && y >= 0 && y < MAXY && z >= 0 && z < MAXZ){
		volume[idx(x, y, z)] = '_';
	}
}
void drawLine(char* volume, int x1, int y1, int z1, int x2, int y2, int z2){
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
                	drawPt(volume, y1, -x1, z1+completion*(z2-z1));
		}else{
                	drawPt(volume, x1, y1, z1+completion*(z2-z1));
		}
                err+=slope;
                if(fabs(err)>=1){
                	y1+=sign;
                	err-=sign;
                }
	}
}
