# subtractiveModel

## OVERVIEW
Using turntable and single webcam, subtractively model objects on inverted backgrounds.

## BUILDING
From base directory:
```
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
``` 

## RUNNING
`./subtractiveModel VIDEO_SRC_IDX FRAMES DIST_TO_CENTER VOLUME_SIDE_LENGTH MODEL_RESOLUTION ITER_LIMIT`
* `VIDEO_SRC_IDX`: OpenCV camera ID. If you only have one camera, specify 0.
* `FRAMES`: The number of silhouette images you want to grab.
* `DIST_TO_CENTER`: The distance from the camera to the center of the object you are rotating. This is arbitrary units, but must be the same as 'VOLUME_SIDE_LENGTH'.
* `VOLUME_SIDE_LENGTH`: What size is the object you are imaging? This parameter specifies the length of one edge of a bounding cube centered at the camera height and axis of rotation.
* `MODEL_RESOLUTION`: How detailed do you want your model? Higher values are more resolution in the final model, but take longer to process.
* `ITER_LIMIT`: How many iterations should it try to correct for imprecise camera placement? It will eventually end on its own, but if you want to set the number of iterations, you can. Otherwise, 1000 is a safe bet.

EX. `./subtractiveModel 0 12 30 10 500 1000`

## BASIC HOW TO USE:
* Set up a softly lit background. A sheet of paper with a lamp shining on it works well.
* Set up the target model on clear supports (A glass cup is good for small objects).
* Run the program.
* You should see a window with webcam output. Pink regions are dark enough to be the model. Adjust your room lighting until the silhouette is mostly just on the model. It can work with some pink around the top, bottom, or sides, but eliminate as much as you can.
* Be sure the webcam center view axis is **intersecting** and **orthogonal** to the model rotation axis. The pink dot should be in the center of the screen.
* For best results, an angle-indexed turntable is recommended.
* Follow the instructions in the terminal window (Beginning with pressing enter when at 0 degrees).
* STL output is written to output.stl.
