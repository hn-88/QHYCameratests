/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * Press s key to capture first frame
 * Press d key to capture second frame and subtract from first
 * as well as calculate mean and std deviation etc.
 * 
 * Hari Nandakumar
 * 6 Jan 2018
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
    unsigned int w,h,bpp,channels,capturenumber;
    unsigned char *ImgData;
    int camtime = 100000,camgain = 1,camspeed = 2,cambinx = 1,cambiny = 1,usbtraffic = 0;
    int camgamma = 1;  
    w=320;
    h=240;

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
            ret = SetQHYCCDBitsMode(camhandle,16);
            if(ret != QHYCCD_SUCCESS)
            {
                //printf("SetQHYCCDParam CONTROL_GAIN failed\n");
                printf("SetQHYCCDBitsMode failed\n");
                
                getchar();
                return 1;
            }

            /*ret = GetQHYCCDChipInfo(camhandle,&chipw,&chiph,&w,&h,&pixelw,&pixelh,&bpp);
                if(ret == QHYCCD_SUCCESS)
                {
                  printf("Chip bit depth is now %d\n", bpp);
                }*/
                     
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
        cvNamedWindow("show",0); // 0 = WINDOW_NORMAL
        cvMoveWindow("show", 20, 0);
        double mean1, mean2, std1, std2;
        Scalar scMEAN, scSTD;
        Mat m, m1, m2;
        //Mat mask(h, w, CV_16U, cv::Scalar(255));
        #endif

      
        
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
                if (key == 115) // s key
                {
					img1 = cvCreateImageHeader(cvSize(w,h),bpp,1);
                    img1->imageData = (char *)ImgData;
                    m1=cvarrToMat(img1, true);
                    m=m1;
                    m.reshape(0,1); // 0=number of channels unchanged, 1=number of rows in reshaped array
                    meanStdDev(m, scMEAN, scSTD);
                    mean1=scMEAN[0];
                    std1=scSTD[0];
                    printf("Mean of 1 is %f \n", mean1);
                    printf("St Dev of 1 is %f \n", std1);
                    cvSaveImage("saved1.png",img1);
                    key=9; // to prevent the next few frames from also going to m1
                    
			    }
			    
			    if (key == 100) // d key
                {
					img2 = cvCreateImageHeader(cvSize(w,h),bpp,1);
                    img2->imageData = (char *)ImgData;
                    m2=cvarrToMat(img2, true);
                    m=m2;
                    m.reshape(0,1); // 0=number of channels unchanged, 1=number of rows in reshaped array
                    meanStdDev(m, scMEAN, scSTD);
                    mean2=scMEAN[0];
                    std2=scSTD[0];
                    printf("Mean of 2 is %f \n", mean2);
                    printf("St Dev of 2 is %f \n", std2);
                    cvSaveImage("saved2.png",img1);
                   
                    //subtract(m2,m1,m, mask, CV_16S); // m=m2-m1
                    m=m2-m1;
                    m=abs(m);
                    m.reshape(0,1); // 0=number of channels unchanged, 1=number of rows in reshaped array
                    meanStdDev(m, scMEAN, scSTD);
                    mean2=scMEAN[0];
                    std2=scSTD[0];
                    printf("Mean of subtracted is %f \n", mean2);
                    printf("St Dev of subtracted is %f \n", std2);
                    cvSaveImage("savedsub.png",img);
                    key=9; // to prevent the next few frames from also going to m2
                    
                    
			    }
			
		 
    
                key=cvWaitKey(3);
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
