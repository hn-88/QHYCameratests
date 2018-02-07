/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * Press s key to capture first frame
 *   second frame and subtract from first
 * as well as calculate mean and std deviation etc.
 * 
 * Hari Nandakumar
 * 9 Jan 2018
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
    
    std::cout << "Please enter number of frames to average: ";
    std::cin >> firstaccum;
    secondaccum = firstaccum;
    numofframes = firstaccum;
    
    std::cout << "Please enter bit depth - 8 or 16: ";
    std::cin >> cambitdepth;
    
    std::cout << "Please keep s key pressed to test.\n ";
    std::cout << "Output goes to output.csv. Press ESC to end program preview window. \n ";

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
        cvNamedWindow("show",0); // 0 = WINDOW_NORMAL
        cvMoveWindow("show", 20, 0);
        double mean1, mean2, std1, std2;
        Scalar scMEAN, scSTD;
        Mat m, m1, m2, msub;
         
        if (cambitdepth==8)
        {
			m1 = Mat::zeros(cv::Size(1, firstaccum), CV_8U);
			m2 = Mat::zeros(cv::Size(1, firstaccum), CV_8U);
		}
        else // is 16 bit
        {
			m1 = Mat::zeros(cv::Size(1, firstaccum), CV_16U);
			m2 = Mat::zeros(cv::Size(1, firstaccum), CV_16U);
		}
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
					m=cvarrToMat(img, true);
					
					if (firstaccum>0)
					{
						 
						if (cambitdepth==8)
						{
							m1.at<uchar>(firstaccum-1)=m.at<uchar>(3,3);
						} else // is 16 bit, so ushort
						{
							m1.at<ushort>(firstaccum-1)=m.at<ushort>(3,3);
						}
						firstaccum--;
						//std::cout<<"Firstaccum ="<<firstaccum<<std::endl;
						//std::cout<<"m1 ="<<m1<<std::endl;
					    
					}
					if (firstaccum==0)
					    {
							if (secondaccum>0)
							{
								 
								if (cambitdepth==8)
								{
									m2.at<uchar>(secondaccum-1)=m.at<uchar>(3,3);
								} else // is 16 bit, so ushort
								{
									m2.at<ushort>(secondaccum-1)=m.at<ushort>(3,3);
								}
								secondaccum--;
							}
						}
					if (secondaccum==0)
								{
									absdiff(m1,m2, msub);
									
									//std::cout<<"m1="<<m1<<std::endl;
									//std::cout<<"m2="<<m2<<std::endl;
									//std::cout<<"|m1-m2|="<< msub <<std::endl;
									meanStdDev(m1, meanpix1, stdpix1);
									meanStdDev(m2, meanpix2, stdpix2);
									averagedsub=meanpix1-meanpix2;
									meanStdDev(msub, meansubtracted, stdsubtracted);
									outfile<<"Gain,Exposure time (us),Camera,Number of frames averaged,Bit depth,Single Pixel Mean1,Single Pixel Mean2,Single Pixel Std Dev,mean of abs(Subtracted),std dev of abs(Subtracted),|mean1 - mean2|"<<std::endl;
									outfile<<camgain<<","<<camtime<<","<<id<<","<<numofframes<<","<<cambitdepth<<","<<meanpix1(0)<<","<<meanpix2(0)<<","<<stdpix1(0)<<","<<meansubtracted(0)<<","<<stdsubtracted(0)<<","<<averagedsub(0)<<std::endl;
									//std::cout<<"mean|m1-m2|="<< meansubtracted(0) <<std::endl; //only the first element is of interest for us, rest zeros
									//std::cout<<"std|m1-m2|="<< stdsubtracted(0) <<std::endl;
									std::cout<<"Done!";
									doneflag=1;
								}
							
							
						
				}
					
				if(doneflag==1)
				break;
    
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
