What the project does?

1. Open camera feed.

2. Create two workers: w1 and w2. Each worker shall perform its work in a separate thread.

3. Create two windows for updates from W1 and W2. 

    a. Worker w1:

        - Process every 2nd frame.

	- W1 queue up to 10 frames. If the queue is full, the newest frames overwrite the oldest frame.

        - Rotate the image clockwise 90 deg. after every 5 frames pushed (1st-5th frame: no rotation, 6th-10th: 90 deg., 11th-15th: 180 deg., 16th-20th: 270 deg., 21th-25th: no rotation etc.). Display the image in gray scale.

     b. Worker w2:

         - Process incoming frames as close to every 1 second (but never more often) as possible.

         - W2 only process 1 frame at a time, if frame is pushed when processing is in progress, incoming frame shall be dropped.

         - Display a mirrored image if the mean pixel intensity of the left half of the frame is above half of the insensitivity range, otherwise display the original image.





Build from console:


git clone https://github.com/vnikanau/CV_assignment.git

cmake -S "CV_assignment" -B "build" -G "Unix Makefiles"

cd build

make




Run:

If you run in Mac Os you can see a dialog to access the camera. Allow access and run the application again.

Press "Esc" for exit




Dependencies:

OpenCV 3.0 or newer (tested with 4.5.3)

C++17

CMake 2.8.12 or newer (tested with 3.12.2)




Checked on:

MacOS 10.15.7 XCode 12.4 

MacOS 10.15.7 Qt Creator 4.2.1 

MacOS 10.15.7 build from console Unix Makefiles

