The .cpp files are meant to be compiled after QHYCCD libraries, OpenCV libraries and USB libraries have been installed.

A known way of compiling them is to put them in the QHYCCD_Linux-master/sample folder, and then
mk build
cd build
cmake ..
make

The existing CMakeLists.txt can be backed up, and the one in this project can be used instead.

Some more documentation is at

http://hnsws.blogspot.in/2017/11/compiling-qhyccd-camera-sdk-sample.html
http://hnsws.blogspot.in/2017/11/installing-latest-opencv-with-python.html

Please note that these .cpp files may need work with compiling on other versions of OpenCV. 
For instance, deprecated C style API calls are used in some places. Please see
http://answers.opencv.org/question/182951/meanstddev-seems-to-have-a-bug-or-am-i-using-it-wrong/
for suggested changes by berak.

//get rid of the IplImage !

Mat image(960, 1280, CV_16U); // allocate data
// now read it in directly
ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels, image.data); 

