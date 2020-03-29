#ifndef VOXTREE_H
#define VOXTREE_H
class Voxtree;
class Voxnode;
typedef struct plane{
	double normal[3];
	double offset;
}plane;
class Voxtree{
public:
	Voxtree(int size);
	static int equalOrGreaterPow2(int target);
	int get(int x, int y, int z);
	int size;
	void deleteLineIntersections(int x, int y, int z, double *v);
	void deletePyramidIntersections(int x, int y, int z, double **v);
	int doesPyramidIntersectFull(int x, int y, int z, double **v);
private:
	Voxnode* root = NULL;
};
class Voxnode{
public:
	Voxnode(int size);
	~Voxnode();
	short size;//FIXME
	char type = 1;//1 is filled. 0 is partial.
	int get(int x, int y, int z);
	int quadrant(int x, int y, int z);
	void quadCoordTrans(int quad, int x, int y, int z, int* ox, int* oy, int* oz);
	int deleteLineIntersections(int x, int y, int z, double *v);
	int deletePyramidIntersectionsRec(int x, int y, int z, double** v, plane* walls);
	int doesPyramidIntersectFullRec(int x, int y, int z, double **v, plane* walls);
	int lineIntersects(int x, int y, int z, double* v);
	int pyramidIntersects(int x, int y, int z, double** v, plane* walls);
private:
	Voxnode *sub[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
};
#endif
