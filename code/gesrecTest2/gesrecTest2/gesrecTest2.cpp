// gesrecTest2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

int testContourTemplateMatch(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;

	IplImage* sampleImg = 0;//样本图片
	IplImage* templateImg = 0;
	IplImage* templateImgOut = 0;
	IplImage* conImg = 0;//轮廓图片
	IplImage* templateResult = 0;
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvMemStorage* templateSto;
	CvSeq* templateContour;

	int isTemp = 0;
	int matching = 0;

	if(argc == 2)
	{
		if((sampleImg = cvLoadImage(argv[1], 1)) == 0)
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
	templateSto = cvCreateMemStorage(0);
	templateContour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), templateSto);

	templateImg = cvLoadImage("myskin1.jpg", 1);
	cvReleaseImage(&templateImgOut);
	cvReleaseImage(&templateResult);
	templateImgOut = cvCloneImage(templateImg);
	templateResult = cvCloneImage(templateImg);
	if(argc == 1)
	{
		gesDetectHandRange(templateImg, templateImgOut, comp);
	}
	else if(argc == 2)
	{
		gesDetectHandRange(templateImg, templateImgOut, comp, &s, 1);
	}
	else 
	{
		return -1;
	}
	gesFindContours(templateImgOut, templateResult, &templateContour, templateSto, 1);

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
		if(argc == 1)
		{
			printf("```````````````\n");
			gesDetectHandRange(input, output, comp);
			printf("```````````````\n");
		}
		else if(argc == 2)
		{
			printf("hell\n");
			gesDetectHandRange(input, output, comp, &s, 1);
			printf("hell\n");
		}
		else
		{
			return -1;
		}

		gesMatchContoursTemplate2(output, conImg, templateContour);
		printf("--------\n");
		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);


		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&templateSto);

	cvReleaseImage(&conImg);
	cvReleaseImage(&templateResult);
	cvReleaseImage(&templateImg);
	cvReleaseImage(&templateImgOut);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

int main(int argc, char** argv)
{
	printf("hello world\n");
	testContourTemplateMatch(argc, argv);
	system("pause");
	return 0;
}

