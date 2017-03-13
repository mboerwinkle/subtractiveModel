#ifndef VOXTREE_H
#define VOXTREE_H
class Voxtree;
class Voxnode;
class Voxtree{
public:
	Voxtree(int size);
	int get(int x, int y, int z);
	int dataSize();
	int size;
	void deleteLineIntersections(int x, int y, int z, double *v);
	void deletePyramidIntersections(int x, int y, int z, double *v1, double *v2);
private:
	Voxnode* root = NULL;
};
class Voxnode{
public:
	Voxnode(int size);
	~Voxnode();
	int dataSize();
	int size;
	char type = 1;//1 is filled. 0 is partial.
	int get(int x, int y, int z);
	int quadrant(int x, int y, int z);
	void quadCoordTrans(int quad, int x, int y, int z, int* ox, int* oy, int* oz);
	int deleteLineIntersections(int x, int y, int z, double *v);
	int deletePyramidIntersections(int x, int y, int z, double *v1, double *v2);
	int lineIntersects(int x, int y, int z, double* v);
	int pyramidIntersects(int x, int y, int z, double* v1, double* v2);
private:
	Voxnode *(sub[8]) = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
};
#endif
