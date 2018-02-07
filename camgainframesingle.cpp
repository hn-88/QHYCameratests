/*
 * modified from QHYCCD SDK
 * sample file LiveFrameSample.cpp
 * * variation of pixel intensity with camera gain
 * * as well as calculate mean and std deviation etc.
 * saved to a file
 * 
 * Hari Nandakumar
 * 30 Jan 2018
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libqhy/qhyccd.h>
#include <sys/time.h>
#include <sys/stat.h>
// this is for mkdir
//#include <fstream>
// this was needed after downgrading to opencv 2.4.9
 

#include <opencv2/opencv.hpp>
// used the above include when imshow was being shown as not declared
// removing
// #include <opencv/cv.h>
// #include <opencv/highgui.h>
 

using namespace cv;

double myStdDev(Mat m, Scalar mean, int w, int h)
{
	//meanStdDev seems to have a bug
	//so a workaround for single channel
	
	double dstddev = 0.0;
	double dmean = mean(0); // only one channel
	
	for (int i = 0; i < w*h; i++)
	{
		if (dmean>m.at<ushort>(i))
		dstddev += (dmean - m.at<ushort>(i)) * (dmean - m.at<ushort>(i));
		else
		dstddev += (m.at<ushort>(i)-dmean) * (m.at<ushort>(i)-dmean);
	}
	 
	dstddev = sqrt(dstddev / (w*h));
	return dstddev;				
}

double myStdDev2(Mat m, Scalar mean, int w, int h)
{
	//meanStdDev seems to have a bug
	//so a workaround for single channel
	//this one written for a signed matrix as input
	
	double dstddev = 0.0;
	double dmean = mean(0); // only one channel
	
	for (int i = 0; i < w*h; i++)
	{
		if (dmean>m.at<short>(i))
		dstddev += (dmean - m.at<short>(i)) * (dmean - m.at<short>(i));
		else
		dstddev += (m.at<short>(i)-dmean) * (m.at<short>(i)-dmean);
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
    unsigned int w,h,bpp,channels,capturenumber, cambitdepth=16, numofframes, firstaccum=1, secondaccum;
    unsigned char *ImgData;
    int camtime = 1,camgain = 1,camspeed = 2,cambinx = 1,cambiny = 1,usbtraffic = 10;
    int camgamma = 1;
    Scalar meansubtracted,  meanframe1,  meanframe2 ;
    Scalar meanabssubtracted ;
    // since meanStdDev seems to have a bug, using our own stddev code 
    double stdsubtracted, stdframe1, stdframe2, stdabssubtracted;
    /////////// 
    std::ofstream outfile("output.csv");
    bool doneflag=0;
    w=1280;
    h=960;
    
    int t_start,t_end, fps, key;
    
    
    //std::cout << "Please enter camera gain - an integer 1 to 100: ";
    //std::cin >> camgain;
    //std::cin >> camgain;
    
     
    /*std::cout << "Please enter number of frames to average: ";
    std::cin >> firstaccum;*/
    secondaccum = firstaccum;
    numofframes = firstaccum;
    
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
        // testing meanStdDev bug - in opencv ver 3.3.1
        //	http://answers.opencv.org/question/182951/meanstddev-seems-to-have-a-bug-or-am-i-using-it-wrong/
        /*Mat mtest(960,1280,CV_16U);
		randn(mtest,10000, 500);
		Scalar M,D;
		meanStdDev(mtest,M,D);
		std::cout << M(0) << " " << D(0) << std::endl;
		Mat m2t = mtest.reshape(0,1);
		meanStdDev(m2t,M,D);
		std::cout << M(0) << " " << D(0);*/
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
        
        Mat m, m1, m2, subtractedframe, abssub;
        int length;
        
        /*if (cambitdepth==8)
        {
			m1 = Mat::zeros(cv::Size(1228800, firstaccum), CV_8U); //1280*960=1228800
			m2 = Mat::zeros(cv::Size(1228800, firstaccum), CV_8U);
		}
        else // is 16 bit
        {
			m1 = Mat::zeros(cv::Size(1228800, firstaccum), CV_16U);
			m2 = Mat::zeros(cv::Size(1228800, firstaccum), CV_16U);
		}*/
		
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
        outfile<<"Gain,Exposure time (us),Camera,Number of frames averaged,w,h,Bit depth,Frame Mean1,Frame Mean2,Frame Std Dev1,Frame Std Dev2,mean of Subtractedframe,std dev of Subtractedframe,mean of abs(Subtractedframe),std dev of abs(Subtractedframe)"<<std::endl;
        
        for (camgain=1; camgain<2;camgain++)
        {
		firstaccum=1;
		secondaccum=firstaccum;	
		doneflag = 0;
        ret = SetQHYCCDParam(camhandle, CONTROL_GAIN, camgain); //handle, parameter name, gain (which can be 0..99)
        if(ret == QHYCCD_SUCCESS)
        {
            printf("CONTROL_GAIN = %d success!\n", camgain);
        }
        else
        {
            printf("CONTROL_GAIN fail\n");
            goto failure;
        }
        
        t_start = time(NULL);
        fps = 0;
        
	     
        while(1)
        { 
            ret = GetQHYCCDLiveFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
            
            if (firstaccum==1)
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
            
            if (ret == QHYCCD_SUCCESS)  
            {
                if(img == NULL)
                {
                    img = cvCreateImageHeader(cvSize(w,h),bpp,1);
                    img->imageData = (char *)ImgData;
                }
                cvShowImage("show",img);
                m=cvarrToMat(img, true);
                //std::cout<<m.size();
			    //std::cout<<"That was size of m"<<std::endl;
						
                m=m.reshape(0,1); //makes m a 1228800x1 matrix 
                                // - 0 indicates channels is unchanged, 1 indicates 1 row.
                                
                //std::cout<<m.size();
				//std::cout<<"That was size of m after reshape"<<std::endl;
						
                
                if (firstaccum>0)
					{ 
						m.copyTo(m1); // row index is from 0 to n-1
						firstaccum--;
					}
				
				else if (firstaccum==0)
					    {
							if (secondaccum>0)
							{
								m.copyTo(m2);
								secondaccum--;
							}
						}
					if (secondaccum==0)
								{
									 
						            //std::cout<<m1.row(0)<<std::endl;
						            meanframe1=mean(m1);
									//meanStdDev(m1, meanframe1, stdframe1);
									//this function seems to have a bug
									
									// so writing a function here. 
									stdframe1 = myStdDev(m1, meanframe1, w, h);
									
									meanframe2=mean(m2);
									stdframe2 = myStdDev(m2, meanframe2, w, h);
									
									subtract(m1, m2, subtractedframe, noArray(), CV_16S);
									
									meansubtracted=mean(subtractedframe);
									stdsubtracted=myStdDev2(subtractedframe, meansubtracted, w, h);
									// subtractedframe is of type CV_16S, so one more function myStdDev2 :)
									meanabssubtracted=mean(abs(subtractedframe));
									stdabssubtracted=myStdDev(abs(subtractedframe),meanabssubtracted, w, h);
									
									//outfile<<"Gain,Exposure time (us),Camera,Number of frames averaged,w,h,Bit depth,Frame Mean1,Frame Mean2,Frame Std Dev1,Frame Std Dev2,mean of Subtractedframe,std dev of Subtractedframe,mean of abs(Subtractedframe),std dev of abs(Subtractedframe)"<<std::endl;
									outfile<<camgain<<","<<camtime<<","<<id<<","<<numofframes<<","<<w<<","<<h<<","<<cambitdepth<<","<<meanframe1(0)<<","<<meanframe2(0)<<","<<stdframe1<<","<<stdframe2<<","<<meansubtracted(0)<<","<<stdsubtracted<<","<<meanabssubtracted(0)<<","<<stdabssubtracted<<std::endl;
									//std::cout<<"mean1="<< meanframe1(0) <<"mean2="<< meanframe2(0)<<std::endl; //only the first element is of interest for us, rest zeros
									
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
