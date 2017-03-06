# subtractiveModel
OVERVIEW
Using turntable and single webcam, subtractively model objects on inverted backgrounds.
BUILDING
run ./debugbuild.sh
execute ./out.out
BASIC HOW TO USE:
Set up a softly lit background.
Set up the target model on clear supports (a glass cup works well for me)
Run the program. It looks for your second webcam (to avoid laptop integrated cams). you can change this by changing main.cpp. Change cam(1) to cam(0).
You should see a window with webcam output. Pink regions are dark enough to be the model. Adjust your room lighting or darkThreshold in camera.h until the silhouette is mostly just on the model. It can work with some pink around the top, bottom or sides. But eliminate as much as you can.
Be sure the webcam is pointed level and at the axis of rotation.
I generally set up 45 degree increments so that I can accurately rotate and position my model.
Follow the instructions in the terminal window (Beginning with pressing enter when at 0 degrees).
output.dat is text output of the model.
output.stl is stl output of the model.
