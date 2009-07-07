#include "gesrec.h"

CvHistogram* gesSampleSkin(const IplImage* src, CvRect window)
{
	IplImage* sampleImg;//��������ͼ��
	IplImage* sampleHSV;
	IplImage* sampleHue;
	CvPoint2D32f center;//������������
	CvHistogram* hist;//ֱ��ͼ
	
	int sizes[1] = {256};
	float ranges[1][2] = {{0,360}};
	float** rangesPtr = new float* [1];
	rangesPtr[0] = ranges[0];

	//�õ���������ͼ��
	sampleImg = cvCreateImage(cvSize(window.width, window.height), src->depth, 3);
	center = cvPoint2D32f(window.x+window.width/2, window.y+window.height/2);
	cvGetRectSubPix(src, sampleImg, center);
	
	//�õ�����ͼ���Hue����
	sampleHSV = cvCreateImage(cvGetSize(sampleImg), IPL_DEPTH_8U, 3);
	sampleHue = cvCreateImage(cvGetSize(sampleImg), IPL_DEPTH_8U, 1);
	cvCvtColor(sampleImg, sampleHSV, CV_BGR2HSV);
	cvSplit(sampleHSV, sampleHue, NULL, NULL, NULL);

	//����������ֵ�ֱ��ͼ
	hist = cvCreateHist(1, sizes, CV_HIST_ARRAY, rangesPtr, 1);
	cvCalcHist(&sampleHue, hist);
	
	//�ͷ��ڴ�
	cvReleaseImage(&sampleImg);
	cvReleaseImage(&sampleHSV);
	cvReleaseImage(&sampleHue);
	delete(rangesPtr);

	return hist;
}

int gesDetectHand(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window)
{
	IplImage* srcHSV;
	IplImage* srcHue;
	IplImage* curImg;//��ǰ����Ŀ�
	CvPoint center;//ÿһС�������
	CvHistogram* hist;//ֱ��ͼ
	int boundryX, boundryY;
	int stepX, stepY;

	int sizes[1] = {256};
	float ranges[1][2] = {{0,360}};
	float** rangesPtr = new float* [1];
	rangesPtr[0] = ranges[0];

	//�õ�Դͼ���Hue����
	srcHSV = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	srcHue = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, srcHSV, CV_BGR2HSV);
	cvSplit(srcHSV, srcHue, NULL, NULL, NULL);

	//��ͼ�����ֱ��ͼƥ��
	curImg = cvCreateImage(cvSize(window.width, window.height), IPL_DEPTH_8U, 1);
	center = cvPoint(window.width/2, window.height/2);
	hist = cvCreateHist(1, sizes, CV_HIST_ARRAY, rangesPtr, 1);
	stepX = window.width / 2;
	stepY = window.height / 2;
	boundryX = src->width - stepX;
	boundryY = src->height - stepY;
	for(;center.x < boundryX;center.x += stepX)
	{
		for(;center.y < boundryY;center.y += stepY)
		{
			cvGetRectSubPix(srcHue, curImg, cvPointTo32f(center));
			cvCalcHist(&curImg, hist);
			if(cvCompareHist(histTemp, hist, CV_COMP_CORREL) <= 20)
			{
				cvRectangle(dst, cvPoint(center.x - stepX, center.y - stepY),
					cvPoint(center.x + stepX, center.y + stepY), cvScalar(255, 0, 0), 1);
			}
		}
	}
	
	//�ͷ��ڴ�
	cvReleaseImage(&srcHSV);
	cvReleaseImage(&srcHue);
	cvReleaseImage(&curImg);
	cvReleaseHist(&hist);
	delete(rangesPtr);

	return 0;
}