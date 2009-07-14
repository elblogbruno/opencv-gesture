// gesrecTest.cpp : �������̨Ӧ�ó������ڵ㡣
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
	IplImage* sampleImg;//����ͼƬ
	IplImage* pImg = 0;//Ҫ����ͼƬ
	IplImage* outImg;//����Ľ��ͼƬ
	CvScalar s;
	CvSeq* comp;//��ͨ����
	CvMemStorage* storage;//��̬�ڴ�
	//CvHistogram* hist;
	
	//����ͼ��
	if( argc == 3 && 
		(pImg = cvLoadImage( argv[1], 1)) != 0 &&
		(sampleImg = cvLoadImage(argv[2], 1)) != 0)
	{
		cvNamedWindow( "Image", 1 );//��������
		cvShowImage( "Image", pImg );//��ʾͼ��

		//��ʼ����̬�ڴ�����ͨ����
		storage = cvCreateMemStorage(0);
		comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);

		//�������ͼƬ��ֱ��ͼ
		/*sampleImg = cvLoadImage("skinsample.JPG", 1);
		hist = gesSampleSkin(sampleImg, cvRect(0,0,sampleImg->width,sampleImg->height));

		//�������ͼƬ
		outImg = cvCloneImage(pImg);
		gesDetectHandHistogram(pImg, outImg, hist, cvRect(0,0,sampleImg->width,sampleImg->height));*/

		//�������ͼƬ�ķ�ɫ��Χ
		gesSampleSkinRange(sampleImg, &s);
		sprintf_s(output, "%.2f %.2f %.2f\n", s.val[0], s.val[1], s.val[2]);
		MessageBox(NULL, output, NULL, MB_OK);

		//�������ͼƬ
		outImg = cvCloneImage(pImg);
		gesDetectHandRange(pImg, outImg, storage, comp, &s, 1);
		//gesDetectHandRange(pImg, outImg);

		cvNamedWindow("Output", 1);
		cvShowImage("Output", outImg);

		cvWaitKey(0); //�ȴ�����

		//cvReleaseHist(&hist);
		cvReleaseMemStorage(&storage);
		cvDestroyWindow("Output");
		cvReleaseImage(&outImg);
		cvDestroyWindow( "Image" );//���ٴ���
		cvReleaseImage( &pImg ); //�ͷ�ͼ��
		return 0;
	}

	return -1;
}

int testCamDetectHandRange(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//����ͼƬ
	CvScalar s;
	CvSeq* comp;//��ͨ����
	CvMemStorage* storage;//��̬�ڴ�

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
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);

	//ѭ����׽,ֱ���û���������ѭ����
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
		if(argc == 1)
		{
			gesDetectHandRange(input, output, storage, comp);
		}
		else
		{
			gesDetectHandRange(input, output, storage, comp, &s, 1);
		}

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");

	return 1;
}

int testImgFindContours(int argc, char** argv)
{
	IplImage* pImg = 0;//Ҫ����ͼƬ
	IplImage* outImg;//����Ľ��ͼƬ
	
	//����ͼ��
	if( argc == 2 && 
		(pImg = cvLoadImage( argv[1], 1)) != 0)
	{
		cvNamedWindow( "Image", 1 );//��������
		cvShowImage( "Image", pImg );//��ʾͼ��

		outImg = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);
		gesFindContours(pImg, outImg);

		cvNamedWindow("Output", 1);
		cvShowImage("Output", outImg);

		cvWaitKey(0); //�ȴ�����

		cvDestroyWindow("Output");
		cvReleaseImage(&outImg);
		cvDestroyWindow( "Image" );//���ٴ���
		cvReleaseImage( &pImg ); //�ͷ�ͼ��
		return 0;
	}

	return 0;
}

int testCamFindContours()
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

	//��׽��һ֡�Գ�ʼ�����
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	output = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
	
	//ѭ����׽,ֱ���û���������ѭ����
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
	
	return 0;
}

int getImgFromCAM(int argc, char** argv)
{
	CvCapture* capture = 0;

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
		
	//ѭ����׽,ֱ���û���������ѭ����
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
			if(argc == 2)
			{
				if(!cvSaveImage(argv[1], input))
				{
					printf("Could not save : %s\n", argv[1]);
				}
			}
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("Input");
	
	return 1;
}

int main( int argc, char** argv )
{
	testCamDetectHandRange(argc, argv);
	//getImgFromCAM(argc, argv);

	return 1;
}