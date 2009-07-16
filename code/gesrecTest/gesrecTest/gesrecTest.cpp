// gesrecTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

char output[40];

void histogram(IplImage* src, IplImage* dst);
int testSampleHistogram(int argc, char** argv);
int testImgDetectHandRange(int argc, char** argv);
int testCamDetectHandRange(int argc, char** argv);
int getImgFromCAM(int argc, char** argv);

//显示输入图像在HSV颜色空间的H分量的直方图
void histogram(IplImage* src, IplImage* dst)
{
	CvHistogram* hist;
	int bin_w;
	float max_val;

	hist = gesSampleSkinHistogram(src, cvRect(0,0,src->width,src->height));
	cvZero(dst);
	cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0);
	cvConvertScale(hist->bins, hist->bins, max_val?255./max_val:0., 0);
	bin_w = dst->width/256;

	for(int i = 0;i < 256;i++)
	{
		double val = (cvGetReal1D(hist->bins, i)*dst->height/255);
		CvScalar color = CV_RGB(255, 255, 0);
		cvRectangle(dst, cvPoint(i*bin_w,dst->height), cvPoint((i+1)*bin_w,(int)(dst->height-val)), color, 1, 8, 0);
	}

	cvReleaseHist(&hist);
}

//调用histogram方法的一个具体测试函数
//cmd使用说明:gesrecTest.exe [index = 0] [filename]
int testSampleHistogram(int argc, char** argv)
{
	IplImage* input;//样本图片
	IplImage* output;//直方图片

	if(argc == 3 && 
		(input = cvLoadImage(argv[2], 1)) != 0)
	{
		cvNamedWindow("Input", 1);
		cvNamedWindow("Output", 1);

		output = cvCreateImage(cvSize(320, 200), IPL_DEPTH_8U, 3);
		histogram(input, output);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvWaitKey(0);

		//释放内存
		cvDestroyWindow("Input");
		cvDestroyWindow("Output");
		cvReleaseImage(&input);
		cvReleaseImage(&output);

		return 1;
	}
	return 0;
}

//检测静态图片中的皮肤
//cmd使用说明:gesrecTest.exe [index = 1] [filename:src] ([filename:sample])
int testImgDetectHandRange(int argc, char** argv)
{
	IplImage* sampleImg;//样本图片
	IplImage* input;//要检测的图片
	IplImage* output;//输出的结果图片
	CvScalar s;//样本直方图
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	
	//载入图像
	if( argc >= 3 && 
		(input = cvLoadImage( argv[2], 1)) != 0)
	{
		//初始化动态内存与连通部件
		storage = cvCreateMemStorage(0);
		comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);

		//获得样本图片的肤色范围
		if(argc == 4)
		{
			if((sampleImg = cvLoadImage(argv[3], 1)) != 0)
			{
				gesSampleSkinRange(sampleImg, &s);
				cvReleaseImage(&sampleImg);
			}
			else
			{
				return 0;
			}
		}

		//生成输出图片
		output = cvCloneImage(input);
		if(argc == 4)
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}
		else
		{
			gesDetectHandRange(input, output, comp);
		}

		cvNamedWindow("Input", 1);
		cvShowImage("Input", input);
		cvNamedWindow("Output", 1);
		cvShowImage("Output", output);

		cvWaitKey(0); //等待按键

		cvReleaseMemStorage(&storage);
		cvDestroyWindow("Input");
		cvDestroyWindow("Output" );
		cvReleaseImage(&output);
		cvReleaseImage(&input);
		return 1;
	}

	return 0;
}

//检测摄像头中的皮肤
//cmd使用说明:gesrecTest.exe [index = 2]
int testCamDetectHandRange(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvSeq* contour = 0;//轮廓信息
	CvMemStorage* conSto;//轮廓内存

	if(argc == 3)
	{
		if((sampleImg = cvLoadImage(argv[2], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("Contour", 1);
		
	//初始化动态内存与连通部件
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	conSto = cvCreateMemStorage(0);
	contour = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), conSto);

	//获得第一帧
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);
		if(argc == 2)
		{
			gesDetectHandRange(input, output, comp);
		}
		else
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}

		gesFindContours(output, conImg, conSto, contour);
		
		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);
		if(cvWaitKey(10) >= 0)
		{
			cvSaveImage("Contour.jpg", conImg);
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&conSto);
	cvReleaseImage(&output);
	cvReleaseImage(&conImg);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

//从摄像头中截图
//cmd使用说明:gesrecTest.exe [index = 3] [filename]
int getImgFromCAM(int argc, char** argv)
{
	CvCapture* capture = 0;

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return 0;
	}

	cvNamedWindow("Input", 1);
		
	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		IplImage* input = 0;

		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvShowImage("Input", input);
		if(cvWaitKey(10) >= 0)
		{
			if(!cvSaveImage(argv[2], input))
			{
				printf("Could not save : %s\n", argv[2]);
			}
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("Input");
	
	return 1;
}

//模版匹配法的测试
int testMatchTemplate(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvSeq* contour = 0;//轮廓信息
	CvMemStorage* conSto;//轮廓内存
	CvMemStorage* templateSto;
	CvSeq* templateContour;
	int isTemp = 0;
	int matching = 0;

	if(argc == 3)
	{
		if((sampleImg = cvLoadImage(argv[2], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("Contour", 1);
		
	//初始化动态内存与连通部件
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	conSto = cvCreateMemStorage(0);
	contour = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), conSto);
	templateSto = cvCreateMemStorage(0);
	templateContour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), templateSto);

	//获得第一帧
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);
		if(argc == 2)
		{
			gesDetectHandRange(input, output, comp);
		}
		else
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}

		gesFindContours(output, conImg, conSto, contour, templateContour, templateSto, &isTemp, matching);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);
		if(cvWaitKey(10) >= 20)
		{
			isTemp = 1;
			matching = 1;
		}
		else 
		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&conSto);
	cvReleaseMemStorage(&templateSto);
	cvReleaseImage(&output);
	cvReleaseImage(&conImg);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

int main( int argc, char** argv )
{
	if(argc >= 2)
	{
		if(strcmp(argv[1], "0") == 0)
		{
			if(testSampleHistogram(argc, argv) == 0)
			{
				printf("Could not open file\n");
			}
		}
		else if(strcmp(argv[1], "1") == 0)
		{
			if(testImgDetectHandRange(argc, argv) == 0)
			{
				printf("Could not open file\n");
			}
		}
		else if(strcmp(argv[1], "2") == 0)
		{
			testCamDetectHandRange(argc, argv);
		}
		else if(strcmp(argv[1], "3") == 0)
		{
			getImgFromCAM(argc, argv);
		}
		else if(strcmp(argv[1], "4") == 0)
		{
			testMatchTemplate(argc, argv);
		}
		else
		{
			printf("Command not exists\n");
		}
	}

	return 1;
}