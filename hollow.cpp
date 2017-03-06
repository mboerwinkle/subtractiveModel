#include <stdio.h>
#include <stdlib.h>
extern int MAXX, MAXY, MAXZ;
extern int idx(int x, int y, int z);
void hollow(char* volume){
	for(int x = 1; x < MAXX-1; x++){
	for(int y = 1; y < MAXY-1; y++){
	for(int z = 1; z < MAXZ-1; z++){
		if(volume[idx(x, y, z)] == '_'){
			continue;
		}
		if(volume[idx(x+1, y, z)] == '_' ||
		volume[idx(x-1, y, z)] == '_' ||
		volume[idx(x, y+1, z)] == '_' ||
		volume[idx(x, y-1, z)] == '_' ||
		volume[idx(x, y, z+1)] == '_' ||
		volume[idx(x, y, z-1)] == '_'){
		}else{
			volume[idx(x, y, z)] = '=';
		}
	}}}
}
