#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
extern int MAXX, MAXY, MAXZ;
extern int idx(int x, int y, int z);
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
void makeStl(char* volume){
	int size = 80+4+sizeof(tri)*1600000;
	char *stl = (char*)calloc(size, 1);
	

	uint32_t triCount = 0;
	tri *data = (tri*)(stl+84);
        for(int x = 1; x < MAXX-1; x++){
        for(int y = 1; y < MAXY-1; y++){
        for(int z = 1; z < MAXZ-1; z++){
                if(volume[idx(x, y, z)] == '_' || volume[idx(x, y, z)] == '='){
                        continue;
                }
		if(volume[idx(x+1, y, z)] == '_'){
			makeXFace(&(data[triCount]), x+1, y, z, 0, 1);
			triCount+=2;
		}
		if(volume[idx(x-1, y, z)] == '_'){
			makeXFace(&(data[triCount]), x, y, z, 1, 0);
			triCount+=2;
		}
		if(volume[idx(x, y+1, z)] == '_'){
			makeYFace(&(data[triCount]), x, y+1, z, 0, 1);
			triCount+=2;
		}
		if(volume[idx(x, y-1, z)] == '_'){
			makeYFace(&(data[triCount]), x, y, z, 1, 0);
			triCount+=2;
		}
		if(volume[idx(x, y, z+1)] == '_'){
			makeZFace(&(data[triCount]), x, y, z+1, 0, 1);
			triCount+=2;
		}
		if(volume[idx(x, y, z-1)] == '_'){
			makeZFace(&(data[triCount]), x, y, z, 1, 0);
			triCount+=2;
		}
		if(triCount%1000 == 0) printf("%d/%ld triangles\n", triCount, (size-84)/sizeof(tri));
	}}}
	printf("%d/%ld triangles\n", triCount, (size-84)/sizeof(tri));
	*((uint32_t*)(stl+80)) = triCount;
	FILE* stlfp = fopen("output.stl", "w");
	fwrite(stl, 1, size, stlfp);
	fclose(stlfp);
	free(stl);
}
