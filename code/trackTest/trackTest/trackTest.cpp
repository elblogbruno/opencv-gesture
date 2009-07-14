// gesrecTest.cpp : 定义控制台应用程序的入口点。
//

#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

int testTracking(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	CvScalar s;
	CvSeq* comp = 0;//连通部件
	CvSeq* curr_comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvMemStorage* storage1;//动态内存

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
		
	//初始化动态内存与连通部件
	storage = cvCreateMemStorage(0);
	storage1 = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	curr_comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage1);

	//循环捕捉,直到用户按键跳出循环体

	IplImage* input = 0;

	input = cvQueryFrame(capture);
	if(!input)
	{
		return -1;
	}

	cvReleaseImage(&output);
	output = cvCloneImage(input);
	if(argc == 1)
	{
		gesDetectHandRange(input, output, comp);
	}
	else
	{
		gesDetectHandRange(input, output, comp, &s, 1);
	}

	cvShowImage("Input", input);
	cvShowImage("Output", output);

	while(true) {

		input = cvQueryFrame(capture);

		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);

		if(comp->total == 0) {
			break;
		}

		if(argc == 1) {
			gesTracking(input, output, comp, curr_comp, &s);
		} else {
			gesTracking(input, output, comp, curr_comp, &s, 1);
		}
		printf("hell\n\n");
		cvClearSeq(comp);
		comp = cvCloneSeq(curr_comp);

		cvShowImage("Input", input);
		cvShowImage("Output", output);

		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&storage1);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");

	return 1;
}

int main( int argc, char** argv )
{
	testTracking(argc, argv);

	return 1;
}