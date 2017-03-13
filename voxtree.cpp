#include <stdio.h>
#include <stdlib.h>
#include "voxtree.h"
Voxtree::Voxtree(int size){
	int realSize = 1;
	while(realSize < size){
		realSize*=2;
	}
	printf("Real Size is %d\n", realSize);
	this->size = realSize;
	root = new Voxnode(this->size);
}
void Voxtree::deleteLineIntersections(int x, int y, int z, double *v){
	root->deleteLineIntersections(x, y, z, v);
}
int Voxtree::get(int x, int y, int z){
	if(x >= size || x < 0 || y >= size || y < 0 || z >= size || z < 0){
		return 0;
	}
	return root->get(x, y, z);
}
void Voxtree::rm(int x, int y, int z){
	if(x >= size || x < 0 || y >= size || y < 0 || z >= size || z < 0){
		return;
	}
	root->rm(x, y, z);
}
int Voxtree::dataSize(){
	return root->dataSize();
}
Voxnode::Voxnode(int size){
	this->size = size;
}
Voxnode::~Voxnode(){
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] != (Voxnode*)NULL){
			puts("all nodes should be freed by the time of deletion!");
		}
	}
}
int Voxnode::deleteLineIntersections(int x, int y, int z, double* v){
	if(!lineIntersects(x, y, z, v)){
		return 0;
	}
	if(type == 1){
		if(size == 1){
			return 1;
		}
		type = 0;
		if(size == 2){
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = 1;
			}
		}else{
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = new Voxnode(size/2);
			}
		}
	}
	int nx, ny, nz;
	bool good = false;
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] == NULL) continue;
		quadCoordTrans(quadIdx, x, y, z, &nx, &ny, &nz);
		if(sub[quadIdx]->deleteLineIntersections(nx, ny, nz, v)){
			delete sub[quadIdx];
			sub[quadIdx] = NULL;//FIXME needed?
		}else good = true;
	}
	if(good == false){
		return 1;
	}
	return 0;
}
int Voxnode::lineIntersects(int x, int y, int z, double* v){
	int c[3] = {x, y, z};
	for(int s = 0; s < 3; s++){//start orientation
		if(v[s] == 0) continue;
		double tInit = -(double)c[s]/v[s];
		double tFinal = (double)(size-c[s])/v[s];
		int init1 = v[(s+1)%3]*tInit+c[(s+1)%3];
		int final1 = v[(s+1)%3]*tFinal+c[(s+1)%3];
		int init2 = v[(s+2)%3]*tInit+c[(s+2)%3];
		int final2 = v[(s+2)%3]*tFinal+c[(s+2)%3];
		if( ((init1 < size && init1 >= 0) && (init2 < size && init2 >= 0)) || ((final1 < size && final1 >= 0) && (final2 < size && final2 >= 0))){
			return 1;
		}
	}
	return 0;
}
int Voxnode::get(int x, int y, int z){
	if(x > size || y > size || z > size){//FIXME remove once final
		puts("Fatal Error! get out of bounds.");
	}
	if(type == 1){
		return 1;
	}
	int quad = quadrant(x, y, z);
	if(sub[quad] == NULL){
		return 0;
	}
	quadCoordTrans(quad, x, y, z, &x, &y, &z);//transform to child coordinate range.
	return sub[quad]->get(x, y, z);
	
}
/*int Voxnode::rm(int x, int y, int z){//FIXME level 0 nodes not use pointer memory.
	if(x > size || y > size || z > size){//FIXME remove once final
		printf("Fatal Error! rm out of bounds. %d %d %d %d\n", x, y, z, size);
	}
	if(type == 1){
		if(size == 1){
			return 1;
		}
		type = 0;
		for(int quadIdx = 0; quadIdx < 8; quadIdx++){
			sub[quadIdx] = new Voxnode(size/2);
		}
	}
	int quad = quadrant(x, y, z);
	if(sub[quad] == NULL){
		return 0;//already removed.
	}
	quadCoordTrans(quad, x, y, z, &x, &y, &z);
	if(sub[quad]->rm(x, y, z)){
		delete sub[quad];
		sub[quad] = NULL;//FIXME needed?
	}
	bool good = false;
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] != NULL){
			good = true;
			break;
		}
	}
	if(!good){
		return 1;
	}
	return 0;
}*/
int Voxnode::dataSize(){
	if(type == 1) return sizeof(Voxnode);
	int totalDataSize = sizeof(Voxnode);
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] != NULL){
			totalDataSize+=sub[quadIdx]->dataSize();
		}
	}
	return totalDataSize;
}
void Voxnode::quadCoordTrans(int quad, int x, int y, int z, int* ox, int* oy, int* oz){//FIXME more efficient. precoded values?
	if(quad<4){
		if(quad < 2){
			if(quad == 0){
//0
			}else{
				*oz=z-size/2;
//1
			}
		}else{
			*oy=y-size/2;
			if(quad == 2){
//2
			}else{
				*oz=z-size/2;
//3
			}
		}
	}else{
		*ox=x-size/2;
		if(quad < 6){
			if(quad == 4){
//4
			}else{
				*oz=z-size/2;
//5
			}
		}else{
			*oy=y-size/2;
			if(quad == 6){
//6
			}else{
				*oz=z-size/2;
//7
			}
		}
	}
}
int Voxnode::quadrant(int x, int y, int z){
	if(size == 1) puts("Fatal! quadrant call on size 1!");
	if(x > size || y > size || z > size){//FIXME remove once final
		printf("Fatal Error! quadrant out of bounds. %d %d %d %d\n", x, y, z, size);
	}
	if(x < size/2){
		if(y < size/2){
			if(z < size/2){
				return 0;
			}else{
				return 1;
			}
		}else{
			if(z < size/2){
				return 2;
			}else{
				return 3;
			}
		}
	}else{
		if(y < size/2){
			if(z < size/2){
				return 4;
			}else{
				return 5;
			}
		}else{
			if(z < size/2){
				return 6;
			}else{
				return 7;
			}
		}
	}
}
