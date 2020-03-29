#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "voxtree.h"
struct tri{ 
	float vec[3];
	float p1[3];
	float p2[3];
	float p3[3];
	uint16_t attr;
}__attribute__((packed));
void makeXFace(tri* out, int x, int y, int z, int d1, int d2){
	out->vec[0] = 0;
	out->vec[1] = 0;
	out->vec[2] = 0;
	out->p1[0]=x;
	out->p1[1]=y;
	out->p1[2]=z;
	out->p2[0]=x;
	out->p2[1]=y+d1;
	out->p2[2]=z+d2;
	out->p3[0]=x;
	out->p3[1]=y+d2;
	out->p3[2]=z+d1;
	out= &(out[1]);
	out->vec[0] = 0;
	out->vec[1] = 0;
	out->vec[2] = 0;
	out->p1[0]=x;
	out->p1[1]=y+d2;
	out->p1[2]=z+d1;
	out->p2[0]=x;
	out->p2[1]=y+d1;
	out->p2[2]=z+d2;
	out->p3[0]=x;
	out->p3[1]=y+1;
	out->p3[2]=z+1;
}
void makeYFace(tri* out, int x, int y, int z, int d1, int d2){
	out->vec[0] = 0;
	out->vec[1] = 0;
	out->vec[2] = 0;
	out->p1[0]=x;
	out->p1[1]=y;
	out->p1[2]=z;
	out->p2[0]=x+d1;
	out->p2[1]=y;
	out->p2[2]=z+d2;
	out->p3[0]=x+d2;
	out->p3[1]=y;
	out->p3[2]=z+d1;
	out= &(out[1]);
	out->vec[0] = 0;
	out->vec[1] = 0;
	out->vec[2] = 0;
	out->p1[0]=x+d2;
	out->p1[1]=y;
	out->p1[2]=z+d1;
	out->p2[0]=x+d1;
	out->p2[1]=y;
	out->p2[2]=z+d2;
	out->p3[0]=x+1;
	out->p3[1]=y;
	out->p3[2]=z+1;
}
//FIXME consistent style between funcs
void makeZFace(tri* d, int x, int y, int z, int d1, int d2){
	d[0].vec[0] = 0;
	d[0].vec[1] = 0;
	d[0].vec[2] = 0;
	d[0].p1[0]=x;
	d[0].p1[1]=y;
	d[0].p1[2]=z;
	d[0].p2[0]=x+d1;
	d[0].p2[1]=y+d2;
	d[0].p2[2]=z;
	d[0].p3[0]=x+d2;
	d[0].p3[1]=y+d1;
	d[0].p3[2]=z;
	d[1].vec[0] = 0;
	d[1].vec[1] = 0;
	d[1].vec[2] = 0;
	d[1].p1[0]=x+d2;
	d[1].p1[1]=y+d1;
	d[1].p1[2]=z;
	d[1].p2[0]=x+d1;
	d[1].p2[1]=y+d2;
	d[1].p2[2]=z;
	d[1].p3[0]=x+1;
	d[1].p3[1]=y+1;
	d[1].p3[2]=z;
}
void makeStl(Voxtree* volume){
	int triStep = 100;
	int size = 84+triStep*sizeof(tri);
	int memTriLeft = 100;
	char *stl = (char*)malloc(size);
	uint32_t triCount = 0;
	tri *data = (tri*)(stl+84);
    for(int x = 1; x < volume->size-1; x++){
		for(int y = 1; y < volume->size-1; y++){
			for(int z = 1; z < volume->size-1; z++){
				if(volume->get(x, y, z) != 1){
					continue;
				}
				if(volume->get(x+1, y, z) == 0){
					makeXFace(&(data[triCount]), x+1, y, z, 0, 1);
					triCount+=2;
					memTriLeft-=2;
				}
				if(volume->get(x-1, y, z) == 0){
					makeXFace(&(data[triCount]), x, y, z, 1, 0);
					triCount+=2;
					memTriLeft-=2;
				}
				if(volume->get(x, y+1, z) == 0){
					makeYFace(&(data[triCount]), x, y+1, z, 0, 1);
					triCount+=2;
					memTriLeft-=2;
				}
				if(volume->get(x, y-1, z) == 0){
					makeYFace(&(data[triCount]), x, y, z, 1, 0);
					triCount+=2;
					memTriLeft-=2;
				}
				if(volume->get(x, y, z+1) == 0){
					makeZFace(&(data[triCount]), x, y, z+1, 0, 1);
					triCount+=2;
					memTriLeft-=2;
				}
				if(volume->get(x, y, z-1) == 0){
					makeZFace(&(data[triCount]), x, y, z, 1, 0);
					triCount+=2;
					memTriLeft-=2;
				}
				if(memTriLeft < triStep/2){
					memTriLeft+=triStep;
					size+=triStep*sizeof(tri);
					stl = (char*)realloc(stl, size);
					data = (tri*)(stl+84);
				}
			}
		}
	}
	printf("%d triangles used, %.3lf MiB allocated, %.3lf MiB saved\n", triCount, (double)size/1048576.0, (double)(84+sizeof(tri)*triCount)/1048576.0);
	*((uint32_t*)(stl+80)) = triCount;
	FILE* stlfp = fopen("output.stl", "w");
	fwrite(stl, 1, 84+triCount*sizeof(tri), stlfp);
	fclose(stlfp);
	free(stl);
}
