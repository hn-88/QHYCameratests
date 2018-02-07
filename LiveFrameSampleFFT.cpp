/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * 
 * demo of a real-time FFT plot from live image stream
 * 
 * Hari Nandakumar
 * 27 Jan 2018
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libqhy/qhyccd.h>
#include <sys/time.h>

#define OPENCV_SUPPORT 1

#ifdef OPENCV_SUPPORT

#include <opencv2/opencv.hpp>
// used the above include when imshow was being shown as not declared
// removing
// #include <opencv/cv.h>
// #include <opencv/highgui.h>

#include <opencv2/plot.hpp>
 

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
    unsigned int w,h,bpp,channels;
    unsigned char *ImgData;
    int camtime = 1,camgain = 99,camspeed = 2,cambinx = 1,cambiny = 1,usbtraffic = 0;
    int camgamma = 1;

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
        
        double chipw,chiph,pixelw,pixelh;
        ret = GetQHYCCDChipInfo(camhandle,&chipw,&chiph,&w,&h,&pixelw,&pixelh,&bpp);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("GetQHYCCDChipInfo success!\n");
            printf("CCD/CMOS chip information:\n");
            printf("Chip width %3f mm,Chip height %3f mm\n",chipw,chiph);
            printf("Chip pixel width %3f um,Chip pixel height %3f um\n",pixelw,pixelh);
            printf("Chip Max Resolution is %d x %d,depth is %d\n",w,h,bpp);
        }
        else
        {
            printf("GetQHYCCDChipInfo fail\n");
            goto failure;
        }
        
       ret = IsQHYCCDControlAvailable(camhandle,CONTROL_TRANSFERBIT);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = SetQHYCCDBitsMode(camhandle,8);
            if(ret != QHYCCD_SUCCESS)
            {
                //printf("SetQHYCCDParam CONTROL_GAIN failed\n");
                printf("SetQHYCCDBitsMode failed\n");
                
                getchar();
                return 1;
            }

            ret = GetQHYCCDChipInfo(camhandle,&chipw,&chiph,&w,&h,&pixelw,&pixelh,&bpp);
                if(ret == QHYCCD_SUCCESS)
                {
                  printf("Chip bit depth is now %d\n", bpp);
                }
                     
        }  
        
        w=1280;
        h=12;      

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
        
        ret = SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, camtime); //handle, parameter name, exposure time (which is in ms)
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
        cvNamedWindow("show",0); // 0 = WINDOW_NORMAL
        cvNamedWindow("Plot",0); // 0 = WINDOW_NORMAL
        cvNamedWindow("FFT",0); // 0 = WINDOW_NORMAL
        cvMoveWindow("show", 20, 0);
        cvMoveWindow("Plot", 700, 300);
        cvMoveWindow("FFT", 20, 300);
        Mat ROI;
        Mat plot_result;
        Mat plot_result2;
        Mat data_x( 1, w, CV_64F );
        Mat data_y( 1, w, CV_64F );
        Mat padded;
         
        int m, n;

		for ( int i = 0; i < data_x.cols; i++ )
		{
			double x = ( i - data_x.cols / 2 );
			data_x.at<double>( 0, i ) = x;
			data_y.at<double>( 0, i ) = x * x * x;
		}
        
        #endif

        //ret = QHYCCD_ERROR;
        //while(ret != QHYCCD_SUCCESS)
         
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
                cvShowImage("show",img);
                ROI= Mat (h,w,CV_8UC1,ImgData,cv::Mat::AUTO_STEP); // width and height are exchanged
                //ROI=ROI(Rect(0,120,w,10)) ; //Rect(x,y,w,h)
                reduce(ROI, data_y, 0, CV_REDUCE_AVG, CV_64F); 
                
                          // source, destination, whether becomes single row or column, type of reduce, type of output
                //imshow("Region",ROI);
                //cvTranspose(ROI,ROI);
                //ROI.convertTo(data_y, CV_64F);
                Ptr<plot::Plot2d> plot = plot::Plot2d::create( data_x, -data_y );
                //plot has y going downwards by default, so -data_y
                plot->setMinY(-255.0);
                plot->setMaxY(0.0);
                plot->render(plot_result);
                imshow( "Plot", plot_result );
                
                // FFT
                
                m = getOptimalDFTSize( data_y.rows );
                n = getOptimalDFTSize( data_y.cols );
                copyMakeBorder(data_y, padded, 0, m - data_y.rows, 0, n - data_y.cols, BORDER_CONSTANT, Scalar::all(0));

				Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
				Mat complexI;
				merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

				dft(complexI, complexI);            // this way the result may fit in the source matrix

				// compute the magnitude and switch to logarithmic scale
				// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
				split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
				magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
				Mat magI = planes[0];

				magI += Scalar::all(1);                    // switch to logarithmic scale
				log(magI, magI);
				//printf("magI rows = %d cols = %d,\n",magI.rows,magI.cols);
				magI.convertTo(magI, CV_64F); // since plot wants CV_64F
				int cx = magI.cols/2;
				Mat q0(magI, Rect(0, 0, cx, 1)); 
				Mat q0x(data_x, Rect(0, 0, cx, 1));
				
				Ptr<plot::Plot2d> plotfft = plot::Plot2d::create( q0x, -q0 );
                //plot has y going downwards by default, so -magI
                //plot->setMinY(-255.0);
                //plot->setMaxY(0.0);
                plotfft->render(plot_result2);
                imshow( "FFT", plot_result2 );
				//
				/* 
				 * // crop the spectrum, if it has an odd number of rows or columns
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

				normalize(magI, magI, 0, 1, CV_MINMAX);*/
				//imshow( "FFT", magI );
				
				///////////////////////////////////
    
                key=cvWaitKey(3);
                if (key == 27)
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
