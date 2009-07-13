// gesrecTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

char output[40];

void histogram(IplImage* pImg, IplImage* histImg);
int testImgDetectHandRange(int argc, char** argv);
int testCamDetectHandRange();
int testImgFindContours(int argc, char** argv);

void histogram(IplImage* pImg, IplImage* histImg)
{
	CvHistogram* hist;
	int bin_w;
	float max_val;

	hist = gesSampleSkinHistogram(pImg, cvRect(0,0,pImg->width,pImg->height));
	cvZero(histImg);
	cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0);
	cvConvertScale(hist->bins, hist->bins, max_val?255./max_val:0., 0);
	bin_w = histImg->width/256;

	for(int i = 0;i < 256;i++)
	{
		double val = (cvGetReal1D(hist->bins, i)*histImg->height/255);
		CvScalar color = CV_RGB(255, 255, 0);
		cvRectangle(histImg, cvPoint(i*bin_w,histImg->height), cvPoint((i+1)*bin_w,(int)(histImg->height-val)), color, 1, 8, 0);
	}

	cvReleaseHist(&hist);
}

int testImgDetectHandRange(int argc, char** argv)
{
	IplImage* sampleImg;//样本图片
	IplImage* pImg = 0;//要检测的图片
	IplImage* outImg;//输出的结果图片
	CvScalar s;
	//CvHistogram* hist;
	
	//载入图像
	if( argc == 3 && 
		(pImg = cvLoadImage( argv[1], 1)) != 0 &&
		(sampleImg = cvLoadImage(argv[2], 1)) != 0)
	{
		cvNamedWindow( "Image", 1 );//创建窗口
		cvShowImage( "Image", pImg );//显示图像

		//获得样本图片的直方图
		/*sampleImg = cvLoadImage("skinsample.JPG", 1);
		hist = gesSampleSkin(sampleImg, cvRect(0,0,sampleImg->width,sampleImg->height));

		//生成输出图片
		outImg = cvCloneImage(pImg);
		gesDetectHandHistogram(pImg, outImg, hist, cvRect(0,0,sampleImg->width,sampleImg->height));*/

		//获得样本图片的肤色范围
		gesSampleSkinRange(sampleImg, &s);
		sprintf_s(output, "%.2f %.2f %.2f\n", s.val[0], s.val[1], s.val[2]);
		MessageBox(NULL, output, NULL, MB_OK);

		//生成输出图片
		outImg = cvCloneImage(pImg);
		gesDetectHandRange(pImg, outImg, &s, 1);
		//gesDetectHandRange(pImg, outImg);

		cvNamedWindow("Output", 1);
		cvShowImage("Output", outImg);

		cvWaitKey(0); //等待按键

		//cvReleaseHist(&hist);
		cvDestroyWindow("Output");
		cvReleaseImage(&outImg);
		cvDestroyWindow( "Image" );//销毁窗口
		cvReleaseImage( &pImg ); //释放图像
		return 0;
	}

	return -1;
}

int testCamDetectHandRange()
{
	CvCapture* capture = 0;
	IplImage* output = 0;

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
		
	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		IplImage* input = 0;

		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);
		gesDetectHandRange(input, output);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");

	return 1;
}

int testImgFindContours(int argc, char** argv)
{
	IplImage* pImg = 0;//要检测的图片
	IplImage* outImg;//输出的结果图片
	
	//载入图像
	if( argc == 2 && 
		(pImg = cvLoadImage( argv[1], 1)) != 0)
	{
		cvNamedWindow( "Image", 1 );//创建窗口
		cvShowImage( "Image", pImg );//显示图像

		outImg = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);
		gesFindContours(pImg, outImg);

		cvNamedWindow("Output", 1);
		cvShowImage("Output", outImg);

		cvWaitKey(0); //等待按键

		cvDestroyWindow("Output");
		cvReleaseImage(&outImg);
		cvDestroyWindow( "Image" );//销毁窗口
		cvReleaseImage( &pImg ); //释放图像
		return 0;
	}

	return 0;
}

int main( int argc, char** argv )
{
	CvCapture* capture = 0;
	IplImage* output = 0;
	IplImage* input = 0;
	
	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}
	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);

	//捕捉第一帧以初始化输出
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	output = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
	
	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvZero(output);
		gesFindContours(input, output);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvReleaseImage(&output);

	return 1;
}