#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "voxtree.h"
//int totalNodeCount = 0;
//int peakNodeCount = 0;
extern void norm(double* v);
extern void cross(double* tv, double* v1, double* v2);
extern double dot(double* v1, double* v2);


Voxtree::Voxtree(int size){
	int realSize = equalOrGreaterPow2(size);
	//printf("Voxel cube dimension: %d\n", realSize);
	this->size = realSize;
	root = new Voxnode(this->size);
}
Voxtree::~Voxtree(){
	delete root;
}
int Voxtree::equalOrGreaterPow2(int target){
	int realSize = 1;
	while(realSize < target){
		realSize*=2;
	}
	return realSize;
}
void Voxtree::deleteLineIntersections(int x, int y, int z, double *v){
	root->deleteLineIntersections(x, y, z, v);
}
void Voxtree::deletePyramidIntersections(int x, int y, int z, double **v){
	plane walls[4];
	for(int idx = 0; idx < 4; idx++){
		cross(walls[idx].normal, v[idx], v[(idx+1)%4]);
		norm(walls[idx].normal);//FIXME needed? Yes. Crossproduct length is area of paralello
	}
	root->deletePyramidIntersectionsRec(x, y, z, v, walls);
}
int Voxtree::doesPyramidIntersectFull(int x, int y, int z, double **v){
	plane walls[4];
	for(int idx = 0; idx < 4; idx++){
		cross(walls[idx].normal, v[idx], v[(idx+1)%4]);
		norm(walls[idx].normal);//FIXME needed? Yes. Crossproduct length is area of paralello
	}
	if(!root->pyramidIntersects(x, y, z, v, walls)){//check just top level to check for oob
		return -1;// out of bounds.
	}
	return root->doesPyramidIntersectFullRec(x, y, z, v, walls);
}
int Voxtree::get(int x, int y, int z){
	if(x >= size || x < 0 || y >= size || y < 0 || z >= size || z < 0){
		puts("get out of bounds");
		return 0;
	}
	return root->get(x, y, z);
}
Voxnode::Voxnode(int size){
	/*totalNodeCount++;
	if(totalNodeCount > peakNodeCount){
		peakNodeCount = totalNodeCount;
	}*/
	this->size = size;
}
Voxnode::~Voxnode(){
	//totalNodeCount--;
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] != (Voxnode*)NULL){
			//puts("all nodes should be freed by the time of deletion!");
			delete sub[quadIdx];
		}
	}
}
int Voxnode::deletePyramidIntersectionsRec(int x, int y, int z, double** v, plane* walls){
	int pyInterRes = pyramidIntersects(x, y, z, v, walls);
	if(!pyInterRes){
		return 0;
	}
	if(pyInterRes == 2 || size == 1){//remove full intersections
		return 1;
	}
	if(type == 1){
		type = 0;
		for(int quadIdx = 0; quadIdx < 8; quadIdx++){
			sub[quadIdx] = new Voxnode(size/2);
		}
	}
	int nx, ny, nz;
	bool good = false;
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){
		if(sub[quadIdx] == NULL) continue;
		quadCoordTrans(quadIdx, x, y, z, &nx, &ny, &nz);
		if(sub[quadIdx]->deletePyramidIntersectionsRec(nx, ny, nz, v, walls)){
			delete sub[quadIdx];//Benchmarked. Does not introduce noticable slowdown
			sub[quadIdx] = NULL;//FIXME needed?
		}else good = true;
	}
	if(good == false){
		return 1;
	}
	return 0;
}
int Voxnode::doesPyramidIntersectFullRec(int x, int y, int z, double** v, plane* walls){
	int pyInterRes = pyramidIntersects(x, y, z, v, walls);
	if(!pyInterRes){//If it doesn't intersect us, it doesn't
		return 0;
	}
	if(pyInterRes == 2 || size == 1 || type == 1){//If it intersects us completely, or we are min size, or we are full, then it does.
		return 1;
	}
	int nx, ny, nz;
	for(int quadIdx = 0; quadIdx < 8; quadIdx++){//We are intersected, but we can't guarantee it is a populated part (we are partial)
		if(sub[quadIdx] == NULL) continue;
		quadCoordTrans(quadIdx, x, y, z, &nx, &ny, &nz);
		if(sub[quadIdx]->doesPyramidIntersectFullRec(nx, ny, nz, v, walls)){//if any of our children have a certain intersection, so do we
			return 1;
		}
	}
	return 0;//we were intersected, but it turns out none of our populated children have a real intersection
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
			for(int quadIdx = 0; quadIdx < 8; quadIdx++){
				sub[quadIdx] = new Voxnode(size/2);
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
bool pointIsInsidePyramid(int x, int y, int z, plane* walls){
	double v[3] = {(double)x, (double)y, (double)z};//fixme normalize directly from int.
	norm(v);//needed?
	bool intersects = true;
	for(int wall = 0; wall < 4; wall++){
//		printf("%d %d %d %lf\n", x, y, z, dot(walls[wall].normal, v));
		if(dot(walls[wall].normal, v)>=0){
			intersects = false;
			break;
		}
	}
	if(intersects){
		return true;
	}
	return false;
}
int Voxnode::pyramidIntersects(int x, int y, int z, double** v, plane* walls){
	for(int i = 0; i < 4; i++){//if one of the four pyramid rays intersects, we know it is partial intersection
		if(lineIntersects(x, y, z, v[i])) return 1;
	}
	//count the number of points inside the pyramid rays
	int hasInside = 0;
	int hasOutside = 0;
	if(size == 1){//we do this to induce an early partial intersection match on the smallest nodes because we don't care if they have partial or full intersection.
		hasOutside = 1;
	}
	for(int tx = 0; tx < 2; tx++){//FIXME rework
		double px = -x+tx*size;
		for(int ty = 0; ty < 2; ty++){
			double py = -y+ty*size;
			for(int tz = 0; tz < 2; tz++){
				double pz = -z+tz*size;
				if(pointIsInsidePyramid(px, py, pz, walls)){
					hasInside++;
					if(hasOutside) return 1;//if we have both, normal intersection (partial, create children)
				}else{
					hasOutside++;
					if(hasInside) return 1;//if we have both, ... (same as above)
				}
			}
		}
	}
	if(hasInside){//this implies "&& !hasOutside
		//printf("We saved some time checking for full (size %d)\n", size); //This does trigger.
		return 2;//full intersection
	}
	return 0;
}
int Voxnode::lineIntersects(int x, int y, int z, double* v){
	int c[3] = {x, y, z};
	for(int s = 0; s < 3; s++){//start orientation
		if(v[s] == 0) continue;//fixme bad behavior at horizontal?
		double tInit = -(double)c[s]/v[s];
		double tFinal = (double)(size-c[s])/v[s];
		double init1 = v[(s+1)%3]*tInit+c[(s+1)%3];//inline?
		double final1 = v[(s+1)%3]*tFinal+c[(s+1)%3];
		double init2 = v[(s+2)%3]*tInit+c[(s+2)%3];
		double final2 = v[(s+2)%3]*tFinal+c[(s+2)%3];
		if( ((init1 < size && init1 >= 0) && (init2 < size && init2 >= 0)) || ((final1 < size && final1 >= 0) && (final2 < size && final2 >= 0))){
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
	//*ox = x;//fuck me. Forgot this and got fractal behavior.
	//*oy = y;
	//*oz = z;
	if(quad<4){
		*ox = x;
		if(quad < 2){//fixme remove else statements//fixme replace else statements to prevent double assignment.
			*oy = y;
			if(quad == 0){
				*oz = z;
			}else{
				*oz=z-size/2;
			}
		}else{
			*oy=y-size/2;
			if(quad == 2){
				*oz = z;
			}else{
				*oz=z-size/2;
			}
		}
	}else{
		*ox=x-size/2;
		if(quad < 6){
			*oy = y;
			if(quad == 4){
				*oz = z;
			}else{
				*oz=z-size/2;
			}
		}else{
			*oy=y-size/2;
			if(quad == 6){
				*oz = z;
			}else{
				*oz=z-size/2;
			}
		}
	}
}
int Voxnode::quadrant(int x, int y, int z){
	if(size == 1) puts("Fatal! quadrant call on size 1!");
	if(x > size || y > size || z > size){//FIXME remove once final
		printf("Fatal Error! quadrant out of bounds. %d %d %d %d\n", x, y, z, size);
	}
	int s2 = size/2;
	if(x < s2){
		if(y < s2){
			if(z < s2){
				return 0;
			}else{
				return 1;
			}
		}else{
			if(z < s2){
				return 2;
			}else{
				return 3;
			}
		}
	}else{
		if(y < s2){
			if(z < s2){
				return 4;
			}else{
				return 5;
			}
		}else{
			if(z < s2){
				return 6;
			}else{
				return 7;
			}
		}
	}
}
