/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * calculate mean and std deviation etc.
 * 
 * Bug in OpenCV or maybe due to some other reason
 * http://answers.opencv.org/question/182951/meanstddev-seems-to-have-a-bug-or-am-i-using-it-wrong/
 * 
 * Hari Nandakumar
 * 28 Jan 2018
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libqhy/qhyccd.h>
#include <sys/time.h>
#include <sys/stat.h>
// this is for mkdir

 

#include <opencv2/opencv.hpp>
// used the above include when imshow was being shown as not declared
// removing
// #include <opencv/cv.h>
// #include <opencv/highgui.h>
 

using namespace cv;
//using namespace std;

int main(int argc,char *argv[])
{
    
        // testing meanStdDev bug - in opencv ver 3.3.1
        //	http://answers.opencv.org/question/182951/meanstddev-seems-to-have-a-bug-or-am-i-using-it-wrong/
        Mat m(9600,1280,CV_16U);
		randn(m,10000, 500);
		Scalar M,D;
		meanStdDev(m,M,D);
		std::cout << M(0) << " " << D(0) << std::endl;
		Mat m2 = m.reshape(0,1);
		meanStdDev(m2,M,D);
		std::cout << M(0) << " " << D(0);
         
}
