/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * and later from Testcam.cpp
 * 
 * * variation of pixel intensity with exposure
 * * as well as calculate mean and std deviation etc.
 * with averaging over multiple frames
 * saved to a file output.csv
 * 
 * Hari Nandakumar
 * 10 Feb 2018
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

double myStdDev(Mat m, Scalar mean, int w, int h)
{
	//meanStdDev seems to have a bug in opencv 3.3.1 & 3.4.0
	//so a workaround for single channel
	
	// Caution! currently implemented only for *single channel* Mat
	// of type CV_8U, CV_8S, CV_16U, CV_16S and CV_64F only!!
	
	double dstddev = 0.0;
	double dmean = mean(0); // only one channel
	
	int typeofmat = m.type();
	
	switch (typeofmat)
	{
		case CV_16UC1:
	
			for (int i = 0; i < w*h; i++)
			{
				if (dmean>m.at<ushort>(i))
				dstddev += (dmean - m.at<ushort>(i)) * (dmean - m.at<ushort>(i));
				else
				dstddev += (m.at<ushort>(i)-dmean) * (m.at<ushort>(i)-dmean);
			}
			break;
		
		case CV_16SC1:
		
			for (int i = 0; i < w*h; i++)
			{
				if (dmean>m.at<short>(i))
				dstddev += (dmean - m.at<short>(i)) * (dmean - m.at<short>(i));
				else
				dstddev += (m.at<short>(i)-dmean) * (m.at<short>(i)-dmean);
			}
			break;
		
		case CV_8UC1:
			
			for (int i = 0; i < w*h; i++)
			{
				if (dmean>m.at<uchar>(i))
				dstddev += (dmean - m.at<uchar>(i)) * (dmean - m.at<uchar>(i));
				else
				dstddev += (m.at<uchar>(i)-dmean) * (m.at<uchar>(i)-dmean);
			}
			break;
		
		case CV_8SC1:
		
			for (int i = 0; i < w*h; i++)
			{
				if (dmean>m.at<char>(i))
				dstddev += (dmean - m.at<char>(i)) * (dmean - m.at<char>(i));
				else
				dstddev += (m.at<char>(i)-dmean) * (m.at<char>(i)-dmean);
			}
			break;
		
		case CV_64FC1:
		
			for (int i = 0; i < w*h; i++)
			{
				if (dmean>m.at<double>(i))
				dstddev += (dmean - m.at<double>(i)) * (dmean - m.at<double>(i));
				else
				dstddev += (m.at<double>(i)-dmean) * (m.at<double>(i)-dmean);
			}
			break;
		
		default:
		
			dstddev=0;
		
	}

	dstddev = sqrt(dstddev / (w*h));
	return dstddev;				
}


int main(int argc,char *argv[])
{
    int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
    char camtype[16];
    int found = 0;
    unsigned int w,h,bpp,channels,capturenumber, cambitdepth=16, numofframes=100, firstaccum, secondaccum;
    unsigned char *ImgData;
    int camtime = 1,camgain = 1,camspeed = 1,cambinx = 2,cambiny = 2,usbtraffic = 10;
    int camgamma = 1;
    Scalar meansubtracted,  pixv1,  pixv2 ;
    double stdsubtracted, stdpixv1, stdpixv2;
    Scalar meanavgsubtracted,  meanavgframe1,  meanavgframe2;
    double  stdavgsubtracted, stdavgframe1, stdavgframe2; 
    std::ofstream outfile("output.csv");
    bool doneflag=0;
    w=640;
    h=480;
    
    int  fps, key;
    double t_start,t_end;
    
    
    //std::cout << "Please enter camera gain - an integer 1 to 100: ";
    //std::cin >> camgain;
    //std::cin >> camgain;
    
     
    /*std::cout << "Please enter number of frames to average: ";
    std::cin >> firstaccum;*/
    
    firstaccum = numofframes;
    secondaccum = firstaccum;
    
    
    /*
    std::cout << "Please enter bit depth - 8 or 16: ";
    std::cin >> cambitdepth;
    
    std::cout << "Please keep s key pressed to test.\n ";
    std::cout << "Output goes to output.csv. Press ESC to end program preview window. \n ";*/

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
        
        ret = SetQHYCCDBinMode(camhandle,cambinx, cambiny); 
        if(ret == QHYCCD_SUCCESS)
        {
            //printf("SetQHYCCDBinMode success - width = %d !\n", w); 
        }
        else
        {
            printf("SetQHYCCDBinMode fail\n");
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
        
        ret = SetQHYCCDParam(camhandle, CONTROL_GAIN, camgain); //handle, parameter name, gain (which can be 0..99)
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
        IplImage *img = NULL;
         
        cvNamedWindow("show",0); // 0 = WINDOW_NORMAL
        cvMoveWindow("show", 20, 0);
        double mean1, mean2, std1, std2;
        Scalar scMEAN, scSTD;
        Mat m, m1, m2, subtractedpixv, m1avg, m2avg, subtractedavgframe;
        int length;
        
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
		 
		
		length = GetQHYCCDMemLength(camhandle);
       
        
        if(length > 0)
        {
			printf("GetQHYCCDMemLength success!\n");
            ImgData = (unsigned char *)malloc(length);
            memset(ImgData,0,length);
            printf("memset success!\n");
        }
        else
        {
            printf("Get the min memory space length failure \n");
            goto failure;
        }
        
        
        ret = BeginQHYCCDLive(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("BeginQHYCCDLive success!\n");
            key=cvWaitKey(300);
        }
        else
        {
            printf("BeginQHYCCDLive failed\n");
            goto failure;
        }

        /////////////////////////////////////////
        /////////////////////////////////////////
        outfile<<"Gain,Exposure time (us),Camera,Number of frames averaged,Binning (n x n),Bit depth,pix Mean1,pix Mean2,Std Dev1,Std Dev2,pix value of (Avged subtractedpixv),stddev of (Avged subtractedpixv)"<<std::endl;
        for (camtime=0; camtime<10000;camtime=camtime+1000)
        {
		firstaccum=numofframes;
		secondaccum=firstaccum;	
		doneflag = 0;
		
        ret = SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, camtime); //handle, parameter name, exposure time (which is in us)
        if(ret == QHYCCD_SUCCESS)
        {
            printf("CONTROL_EXPOSURE =%d success!\n", camtime);
        }
        else
        {
            printf("CONTROL_EXPOSURE fail\n");
            goto failure;
        }
        t_start = time(NULL);
        fps = 0;
        
	     
        while(1)
        { 
            ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
            
            
            
            if (firstaccum==numofframes)
            {
				// the first two frames after gain change has all white pixels, discard them
				// also need to give a longer wait. 
				key=cvWaitKey(500);
				ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
				key=cvWaitKey(500);
				ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
				key=cvWaitKey(500);
				ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
			}
			
			fps++;
            t_end = time(NULL);
            if(t_end - t_start >= 5)
                {
                    printf("fps = %d\n",fps/(5*numofframes*4));
                     
                    fps = 0;
                    t_start = time(NULL);
                }
            
            if (ret == QHYCCD_SUCCESS)  
            {
                if(img == NULL)
                {
                    img = cvCreateImageHeader(cvSize(w,h),bpp,1);
                    img->imageData = (char *)ImgData;
                }
                cvShowImage("show",img);
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
		
					}
				 
				else if (firstaccum==0)
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
									 
						            
						  
									pixv1=mean(m1);
								
									//meanStdDev(m1, pixv1, stdpixv1);
									//this function seems to have a bug
									
									// so writing a function here. 
									stdpixv1 = myStdDev(m1, pixv1, 1, numofframes);
									
									pixv2=mean(m2);
									stdpixv2 = myStdDev(m2, pixv2, 1, numofframes);
									
									subtract(m1 , m2 , subtractedpixv, noArray(), CV_16S);
									
									meansubtracted=mean(subtractedpixv);
									stdsubtracted=myStdDev(subtractedpixv, meansubtracted, 1, numofframes);
									
									//outfile<<"Gain,Exposure time (us),Camera,Number of frames averaged,Binning (n x n),Bit depth,pix Mean1,pix Mean2,Std Dev1,Std Dev2,pix value of (Avged subtractedpixv),stddev of (Avged subtractedpixv)"<<std::endl;
									outfile<<camgain<<","<<camtime<<","<<id<<","<<numofframes<<","<<cambinx<<","<<cambitdepth<<","<<pixv1(0)<<","<<pixv2(0)<<","<<stdpixv1<<","<<stdpixv2<<","<<meansubtracted(0)<<","<<stdsubtracted<<std::endl;
									std::cout<<"mean1="<< pixv1(0) <<"mean2="<< pixv2(0)<<std::endl; //only the first element is of interest for us, rest zeros
									//std::cout<<"std|m1-m2|="<< stdsubtracted(0) <<std::endl;
									//std::cout<<"Done!";
									doneflag=1;
								}

					 
					 
                key=cvWaitKey(3);
                if (key == 27) // ESC key
                break; 
            } 
            if (key == 27) // ESC key
                break;
                
            if(doneflag==1)
				break;    

        } // while loop end
        if (key == 27) // ESC key
                break;
        
		} // end of for loop
		delete(ImgData);
          
        
        
    } // end of if found
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
