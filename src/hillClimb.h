#ifndef HILLCLIMB_H
#define HILLCLIMB_H
/*
 * performs a hill-climbing search for a set of double values.
 * For value 'idx', it is bounded by min[idx] and max[idx]. Searching should be performed in steps of step[idx].
 * Fitness is determined by calling the fitness function.
 * Extradata is just whatever other data you want to pass to the fitness function.
 * Axes is the order of the search space
 */
extern double* hillClimb(int axes, double* min, double* max, double* step, void* extraData, double (*fitness)(void*, double*), int maxIterations);
#endif
