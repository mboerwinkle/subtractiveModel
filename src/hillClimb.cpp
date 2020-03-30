#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hillClimb.h"

double* hillClimb(int axes, double* min, double* max, double* step, void* extraData, double (*fitness)(void*, double*)){//Fixme save already computed domain stuff.
	double* ret = (double*)calloc(axes, sizeof(double));
	for(int a = 0; a < axes; a++){
		ret[a] = (min[a]+max[a])/2.0;
	}
	double maxScore = fitness(extraData, ret);//Init maxscore to the score at the center of all axes.
	double test[axes];
	int improved = 1;//did we improve this iteration?
	int testAxis = 0;//This keeps us from improving in just one axis the whole time.
	int testDirection = 1;//This keeps us from just improving one direction the whole time.
	while(improved){
		improved = 0;
		int startAxis = testAxis;//This is the axis we are starting with. Once we return here, we know we did the whole thing.
		int startDirection = testDirection;
		do{
			memcpy(test, ret, axes*sizeof(double));//Reset all the axes to the best available
			test[testAxis] += testDirection*step[testAxis];//Change the selected axis
			if(test[testAxis] < min[testAxis] || test[testAxis] > max[testAxis]){
				printf("Warning exceeded hillclimb domain (Axis: %d, Value: %lf)\n", testAxis, test[testAxis]);
			}
			double testScore = fitness(extraData, test);
			printf("(%lf", test[0]);
			for(int pIdx = 1; pIdx < axes; pIdx++){
				printf(" %lf", test[pIdx]);
			}
			if(testScore < 0.0001){
				printf(") Found a result of score %e\n", testScore);
			}else{
				printf(") Found a result of score %lf\n", testScore);
			}
			if(testDirection == 1){//Iterate through the directions and axes
				testDirection = -1;
			}else{
				testDirection = 1;
				testAxis = (testAxis+1)%axes;
			}
			if(testScore > maxScore){//If we improve, overwrite all the max values
				maxScore = testScore;
				memcpy(ret, test, axes*sizeof(double));
				improved = 1;
				printf("Hillclimb: Found a new max score: %lf\n", maxScore);
				break;
			}
		}while(startAxis != testAxis || startDirection != testDirection);
	}
	return ret;
}
