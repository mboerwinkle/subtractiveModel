#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "voxtree.h"
int totalNodeCount = 0;
int peakNodeCount = 0;
extern void norm(double* v);
extern void cross(double* tv, double* v1, double* v2);
extern double dot(double* v1, double* v2);

typedef struct plane{
	double normal[3];
	double offset;
}plane;
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
void Voxtree::deletePyramidIntersections(int x, int y, int z, double **v){
	root->deletePyramidIntersections(x, y, z, v);
}
int Voxtree::get(int x, int y, int z){
	if(x >= size || x < 0 || y >= size || y < 0 || z >= size || z < 0){
		puts("get out of bounds");
		return 0;
	}
	return root->get(x, y, z);
}
Voxnode::Voxnode(int size){
//	if(size > 1){
		totalNodeCount++;
		if(totalNodeCount > peakNodeCount){
			peakNodeCount = totalNodeCount;
		}
	//}
	this->size = size;
}
Voxnode::~Voxnode(){
//	if(size > 1){
		totalNodeCount--;
	//}
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] != (Voxnode*)NULL){
			puts("all nodes should be freed by the time of deletion!");
		}
	}
}
int Voxnode::deletePyramidIntersections(int x, int y, int z, double **v){
	if(!pyramidIntersects(x, y, z, v)){
		return 0;
	}
	if(type == 1){
		if(size == 1){
			return 1;
		}
		type = 0;
/*		if(size == 2){
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = 1;
			}
		}else{*/
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = new Voxnode(size/2);
			}
//		}
	}
	int nx, ny, nz;
	bool good = false;
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] == NULL) continue;
		quadCoordTrans(quadIdx, x, y, z, &nx, &ny, &nz);
		if(sub[quadIdx]->deletePyramidIntersections(nx, ny, nz, v)){
			delete sub[quadIdx];
			sub[quadIdx] = NULL;//FIXME needed?
		}else good = true;
	}
	if(good == false){
		return 1;
	}
	return 0;
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
/*		if(size == 2){
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = 1;
			}
		}else{*/
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = new Voxnode(size/2);
			}
//		}
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
bool pointIsInsidePyramid(int x, int y, int z, plane* walls){
	double v[3] = {(double)x, (double)y, (double)z};//fixme normalize directly from int.
	norm(v);
	bool intersects = true;
	for(int wall = 0; wall < 4; wall++){
//		printf("%d %d %d %lf\n", x, y, z, dot(walls[wall].normal, v));
		if(dot(walls[wall].normal, v)>=0){
			intersects = false;
			break;
			//fixme early exit
		}
	}
	if(intersects){
		return true;
	}
	return false;
}
int Voxnode::pyramidIntersects(int x, int y, int z, double** v){
	for(int i = 0; i < 4; i++){//if one of the four pyramid rays intersects
		if(lineIntersects(x, y, z, v[i])) return 1;
	}
	plane walls[4];
	for(int idx = 0; idx < 4; idx++){
		cross(walls[idx].normal, v[idx], v[(idx+1)%4]);
		norm(walls[idx].normal);//FIXME needed?
/*		if(idx == 0) puts("right");
		if(idx == 1) puts("top");
		if(idx == 2) puts("left");
		if(idx == 3) puts("bottom");
		printf("%lf %lf %lf\n", walls[idx].normal[0], walls[idx].normal[1], walls[idx].normal[2]);
*/
	}

	//or one of the eight points is inside of the pyramid
	for(int tx = 0; tx < 2; tx++){//FIXME rework
		double px = -x+tx*size;
		for(int ty = 0; ty < 2; ty++){
			double py = -y+ty*size;
			for(int tz = 0; tz < 2; tz++){
				double pz = -z+tz*size;
				if(pointIsInsidePyramid(px, py, pz, walls)){
					if(size > 1){
			//			printf("size: %d\n", size);
					}
					return 1;
				}
			}
		}
	}
	return 0;
}
int Voxnode::lineIntersects(int x, int y, int z, double* v){
	int c[3] = {x, y, z};
	for(int s = 0; s < 3; s++){//start orientation
		if(v[s] == 0) continue;//fixme bad behavior at horizontal?
		double tInit = -(double)c[s]/v[s];
		double tFinal = (double)(size-c[s])/v[s];
		double init1 = v[(s+1)%3]*tInit+c[(s+1)%3];
		double final1 = v[(s+1)%3]*tFinal+c[(s+1)%3];
		double init2 = v[(s+2)%3]*tInit+c[(s+2)%3];
		double final2 = v[(s+2)%3]*tFinal+c[(s+2)%3];
		if( ((init1 <= size && init1 >= 0) && (init2 <= size && init2 >= 0)) || ((final1 <= size && final1 >= 0) && (final2 <= size && final2 >= 0))){
			return 1;
		}
	}
	return 0;
}

int Voxnode::get(int x, int y, int z){//works with bottom nodes stored as 1 or 0.
	if(x >= size || x < 0 || y >= size || y < 0 || z >= size || z < 0){
		puts("get out of bounds");
		return 0;
	}
	int quad = quadrant(x, y, z);
	if(type == 1) return 1;//if it is filled, it exists
	if(sub[quad] == NULL) return 0;//if it is not filled, and that child is null, it does not exist
	if(size == 2) return 1;//if it is not filled, and the child exists, and it is size two, then it exists
	quadCoordTrans(quad, x, y, z, &x, &y, &z);//transform to child coordinate range.
	return sub[quad]->get(x, y, z);//otherwise, check the child
	
}
void Voxnode::quadCoordTrans(int quad, int x, int y, int z, int* ox, int* oy, int* oz){//FIXME more efficient. precoded values?
	*ox = x;//fuck me. Forgot this and got fractal behavior.
	*oy = y;
	*oz = z;
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
