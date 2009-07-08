// gesrecTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"

void histogram(IplImage* pImg, IplImage* histImg)
{
	CvHistogram* hist;
	int bin_w;
	float max_val;

	hist = gesSampleSkin(pImg, cvRect(0,0,pImg->width,pImg->height));
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

int main( int argc, char** argv )
{
	IplImage* sampleImg;//����ͼƬ
	IplImage* pImg;//Ҫ����ͼƬ
	IplImage* outImg;//����Ľ��ͼƬ
	CvHistogram* hist;
	
	//����ͼ��
	if( argc == 2 && 
		(pImg = cvLoadImage( argv[1], 1)) != 0 )
	{
		cvNamedWindow( "Image", 1 );//��������
		cvShowImage( "Image", pImg );//��ʾͼ��

		//�������ͼƬ��ֱ��ͼ
		sampleImg = cvLoadImage("skinsample.JPG", 1);
		hist = gesSampleSkin(sampleImg, cvRect(0,0,sampleImg->width,sampleImg->height));

		//�������ͼƬ
		outImg = cvCreateImage(cvGetSize(pImg), pImg->depth, 3);
		outImg = cvCloneImage(pImg);
		gesDetectHandHistogram(pImg, outImg, hist, cvRect(0,0,sampleImg->width,sampleImg->height));

		/*//�������ͼƬ
		outImg = cvCreateImage(cvGetSize(pImg), pImg->depth, 3);
		outImg = cvCloneImage(pImg);
		gesDetectHandRange(pImg, outImg);*/

		cvNamedWindow("Output", 1);
		cvShowImage("Output", outImg);

		cvWaitKey(0); //�ȴ�����

		cvReleaseHist(&hist);
		cvDestroyWindow("Output");
		cvReleaseImage(&outImg);
		cvDestroyWindow( "Image" );//���ٴ���
		cvReleaseImage( &pImg ); //�ͷ�ͼ��
		return 0;
	}

	return -1;
}