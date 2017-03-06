#include <stdio.h>
#include <stdlib.h>
extern int MAXX, MAXY, MAXZ;
int idx(int x, int y, int z){
	return x+y*MAXX+z*MAXX*MAXY;
}
void stitchVoxels(char* volume){
	for(int x = 1; x < MAXX-1; x++){
	for(int y = 1; y < MAXY-1; y++){
	for(int z = 1; z < MAXZ-1; z++){
	if(volume[idx(x, y, z)] == '+') break;
	for(int dx = 0; dx < 2; dx++){
	for(int dy = 0; dy < 2; dy++){
	for(int dz = 0; dz < 2; dz++){
		if(volume[idx(x+dx, y+dy, z+dz)]=='+' && volume[idx(x-dx, y-dy, z-dz)]=='+'){//FIXME
			volume[idx(x, y, z)] = '*';
		}
	}}}}}}
}
