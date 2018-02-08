/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * Shows 2D FFT of input frame
 * 
 * Press s to save. 
 *   
 * For sensor noise studies as mentioned at
 * http://www.qsimaging.com/ccd_noise.html
 * 
 * Hari Nandakumar
 * 8 Feb 2018
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libqhy/qhyccd.h>
#include <sys/time.h>
#include <sys/stat.h>
// this is for mkdir

#define OPENCV_SUPPORT 1

#ifdef OPENCV_SUPPORT

#include <opencv2/opencv.hpp>
// used the above include when imshow was being shown as not declared
// removing
// #include <opencv/cv.h>
// #include <opencv/highgui.h>
 

using namespace cv;

#endif

int main(int argc,char *argv[])
{
    int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
    char camtype[16];
    int found = 0;
    unsigned int w,h,bpp,channels,capturenumber, cambitdepth, numofframes, firstaccum, secondaccum;
    unsigned char *ImgData;
    int camtime = 100,camgain = 1,camspeed = 2,cambinx = 1,cambiny = 1,usbtraffic = 0;
    int camgamma = 1;
    Scalar meansubtracted, stdsubtracted, meanpix1, stdpix1, meanpix2, stdpix2, averagedsub;  
    std::ofstream outfile("output.csv");
    bool doneflag=0;
    w=320;
    h=240;
    
    std::cout << "Please enter camera gain - an integer 1 to 100: ";
    std::cin >> camgain;
    
    std::cout << "Please enter camera exposure time in microseconds - an integer 1 to 1000000: ";
    std::cin >> camtime;
    /*
    std::cout << "Please enter number of frames to average: ";
    std::cin >> firstaccum;
    secondaccum = firstaccum;
    numofframes = firstaccum;
    */
    std::cout << "Please enter bit depth - 8 or 16: ";
    std::cin >> cambitdepth;
    
    std::cout << "Press s key to save FFT.\n ";
    std::cout << "Press ESC to end program preview window. \n ";

    ret = InitQHYCCDResource();
    if(ret != QHYCCD_SUCCESS)
    {
        printf("Init SDK not successful!\n");
    }
    /*else
    {
        goto failure;
    }*/
    num = ScanQHYCCD();
    if(num > 0)
    {
        printf("Yes!Found QHYCCD,the num is %d \n",num);
    }
    else
    {
        printf("Not Found QHYCCD,please check the usblink or the power\n");
        goto failure;
    }

    for(int i = 0;i < num;i++)
    {
        ret = GetQHYCCDId(i,id);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("connected to the first camera from the list,id is %s\n",id);
            found = 1;
            break;
        }
    }

    if(found == 1)
    {
        camhandle = OpenQHYCCD(id);
        if(camhandle != NULL)
        {
            //printf("Open QHYCCD success!\n");
        }
        else
        {
            printf("Open QHYCCD failed \n");
            goto failure;
        }
        ret = SetQHYCCDStreamMode(camhandle,1);
    

        ret = InitQHYCCD(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("Init QHYCCD success!\n");
        }
        else
        {
            printf("Init QHYCCD fail code:%d\n",ret);
            goto failure;
        }
        
        
        
       ret = IsQHYCCDControlAvailable(camhandle,CONTROL_TRANSFERBIT);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDBitsMode(camhandle,cambitdepth);
            if(ret != QHYCCD_SUCCESS)
            {
                printf("SetQHYCCDBitsMode failed\n");
                
                getchar();
                return 1;
            }

           
                     
        }  
              

        ret = SetQHYCCDResolution(camhandle,0,0, w, h); //handle, xpos,ypos,xwidth,ywidth
        if(ret == QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDResolution success - width = %d !\n", w); 
        }
        else
        {
            printf("SetQHYCCDResolution fail\n");
            goto failure;
        }
        
        ret = SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, usbtraffic); //handle, parameter name, usbtraffic (which can be 0..100 perhaps)
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("CONTROL_USBTRAFFIC success!\n");
        }
        else
        {
            printf("CONTROL_USBTRAFFIC fail\n");
            goto failure;
        }
        
        ret = SetQHYCCDParam(camhandle, CONTROL_SPEED, camspeed); //handle, parameter name, speed (which can be 0,1,2)
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("CONTROL_CONTROL_SPEED success!\n");
        }
        else
        {
            printf("CONTROL_CONTROL_SPEED fail\n");
            goto failure;
        }
        
        ret = SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, camtime); //handle, parameter name, exposure time (which is in us)
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("CONTROL_EXPOSURE success!\n");
        }
        else
        {
            printf("CONTROL_EXPOSURE fail\n");
            goto failure;
        }
        
        ret = SetQHYCCDParam(camhandle, CONTROL_GAIN, camgain); //handle, parameter name, gain (which can be 0..100 perhaps)
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("CONTROL_GAIN success!\n");
        }
        else
        {
            printf("CONTROL_GAIN fail\n");
            goto failure;
        }
        
        ret = SetQHYCCDParam(camhandle, CONTROL_GAMMA, camgamma); //handle, parameter name, gamma (which can be 0..2 perhaps)
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("CONTROL_GAMMA success!\n");
        }
        else
        {
            printf("CONTROL_GAMMA fail\n");
            goto failure;
        }
        
        ret = BeginQHYCCDLive(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("BeginQHYCCDLive success!\n");
        }
        else
        {
            printf("BeginQHYCCDLive failed\n");
            goto failure;
        }

        int length = GetQHYCCDMemLength(camhandle);
        
        if(length > 0)
        {
            ImgData = (unsigned char *)malloc(length);
            memset(ImgData,0,length);
        }
        else
        {
            printf("Get the min memory space length failure \n");
            goto failure;
        }
    

        int t_start,t_end;
        t_start = time(NULL);
        int fps = 0;
        int key;
        #ifdef OPENCV_SUPPORT
        IplImage *img = NULL;
        IplImage *img1 = NULL;
        IplImage *img2 = NULL;
        //cvNamedWindow("show",0); // 0 = WINDOW_NORMAL
        //cvMoveWindow("show", 20, 0);
        double mean1, mean2, std1, std2;
        Scalar scMEAN, scSTD;
        Mat m1, I;
        
        I = Mat::zeros(cv::Size(w, h), CV_32F);
         
         
        if (cambitdepth==8)
        {
			m1 = Mat::zeros(cv::Size(w, h), CV_8U);
			 
		}
        else // is 16 bit
        {
			m1 = Mat::zeros(cv::Size(w, h), CV_16U);
			 
		}
        #endif
        /////////////////////////////////////
        // from https://docs.opencv.org/3.2.0/d8/d01/tutorial_discrete_fourier_transform.html
        
        Mat padded;                            //expand input image to optimal size
         
		 int m = getOptimalDFTSize( I.rows );
		 int n = getOptimalDFTSize( I.cols ); // on the border add zero values
		 

      
        
        while(1)
        {
            ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels,ImgData);

            if(ret == QHYCCD_SUCCESS)
            {
                //printf("GetQHYCCDLiveFrame success - width = %d !\n", w);
                #ifdef OPENCV_SUPPORT
                if(img == NULL)
                {
                    img = cvCreateImageHeader(cvSize(w,h),bpp,1);
                    img->imageData = (char *)ImgData;
                }
                //cvShowImage("show",img);
                m1 = cvarrToMat(img, true);
                m1.convertTo(I, CV_32F);
                
                copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));
	 
				Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
				Mat complexI;
                
                
                
                // from https://docs.opencv.org/3.2.0/d8/d01/tutorial_discrete_fourier_transform.html
                
                merge(planes, 2, complexI);         // Add to the expanded another plane with zeros
			 
				 dft(complexI, complexI);            // this way the result may fit in the source matrix
			 
				 // compute the magnitude and switch to logarithmic scale
				 // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
				 split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
				 magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
				 Mat magI = planes[0];
				 Mat magI8bit;
				 
			 
				 magI += Scalar::all(1);                    // switch to logarithmic scale
				 log(magI, magI);
			 
				 // crop the spectrum, if it has an odd number of rows or columns
				 magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));
				 
				 
				 // rearrange the quadrants of Fourier image  so that the origin is at the image center
				 int cx = magI.cols/2;
				 int cy = magI.rows/2;
			 
				 Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
				 Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
				 Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
				 Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right
			 
				 Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
				 q0.copyTo(tmp);
				 q3.copyTo(q0);
				 tmp.copyTo(q3);
			 
				 q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
				 q2.copyTo(q1);
				 tmp.copyTo(q2);
				 
				 
			 
				 normalize(I, I, 0, 1, NORM_MINMAX); // Transform the matrix with float values into a
														 // viewable image form (float between values 0 and 255).
														 
				normalize(magI, magI, 0, 1, NORM_MINMAX); // Transform the matrix with float values into a
														 // viewable image form (float between values 0 and 255).
														 
			 
				imshow("Input Image"       , I   );    // Show the result
				imshow("Spectrum magnitude", magI);
				moveWindow("Input Image", 20, 0);
                
                if (key == 115) // s key
                {
					magI=magI*255;
					I=I*255;
					magI.convertTo(magI8bit, CV_8U);
					//std::cout<<magI8bit;
					imwrite( "savedFFT.png", magI8bit );
					
					I.convertTo(magI8bit, CV_8U);
					imwrite( "savedNoiseImg.png", magI8bit );
					
					    
				}
					 
                key=cvWaitKey(1);
                if (key == 27) // ESC key
                break;
                #endif
                fps++;
                t_end = time(NULL);
                if(t_end - t_start >= 5)
                {
                    printf("fps = %d\n",fps / 5);
                    fps = 0;
                    t_start = time(NULL);
                }
            }

        }
        delete(ImgData);  
    }
    else
    {
        printf("The camera is not QHYCCD or other error \n");
        goto failure;
    }
    
    if(camhandle)
    {
        StopQHYCCDLive(camhandle);

        ret = CloseQHYCCD(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("Close QHYCCD success!\n");
        }
        else
        {
            goto failure;
        }
    }

    ret = ReleaseQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Release SDK Resource  success!\n");
    }
    else
    {
        goto failure;
    }

    return 0;

failure:
    printf("Fatal error !! \n");
    return 1;
}
