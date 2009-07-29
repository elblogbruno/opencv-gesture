// gesrecTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#define TIMES 33

#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

int testTracking(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* output = 0;
	IplImage* detectOutput = 0;
	IplImage* sampleImg;//����ͼƬ
	CvScalar s;
	CvSeq* comp = 0;//��ͨ����
	CvSeq* curr_comp;//��ͨ����
	CvMemStorage* storage;//��̬�ڴ�
	CvMemStorage* storage1;//��̬�ڴ�

	if(argc == 2)
	{
		if((sampleImg = cvLoadImage(argv[1], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//�������ͼƬ�ķ�ɫ��Χ
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
		
	//��ʼ����̬�ڴ�����ͨ����
	storage = cvCreateMemStorage(0);
	storage1 = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	curr_comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage1);

	//ѭ����׽,ֱ���û���������ѭ����

	IplImage* input = 0;
	// ÿTIMES֡����һ�����µļ��
	int i = 0;
	while(true) {
		input = cvQueryFrame(capture);
		if(!input) {
			continue;
		}
		cvReleaseImage(&detectOutput);
		detectOutput = cvCloneImage(input);
		cvReleaseImage(&output);
		output = cvCloneImage(input);
		if( i % TIMES == 0 ) {
			if(argc == 1) {
				gesDetectHandRange(input, detectOutput, comp);
			}
			else {
				gesDetectHandRange(input, detectOutput, comp, &s, 1);
			}
			i = 1;
		}

		if(comp->total == 0 || !comp) {
			i = 0;
		} else {
			if(argc == 1) {
				gesTracking(input, output, comp, curr_comp, &s);
			} else {
				gesTracking(input, output, comp, curr_comp, &s, 1);
			}
			cvClearSeq(comp);
			comp = cvCloneSeq(curr_comp);
			
			i++;
		}

		cvShowImage("Input", input);
		cvShowImage("Output", output);

		if(cvWaitKey(10) >= 0) {
			break;
		}
	}
	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&storage1);

	cvReleaseImage(&output);
	cvReleaseImage(&detectOutput);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");

	return 1;
}

int main( int argc, char** argv )
{
	testTracking(argc, argv);

	return 1;
}