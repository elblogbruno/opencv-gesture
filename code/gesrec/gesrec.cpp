#include "gesrec.h"

CvHistogram* gesSampleSkin(const IplImage* img, CvRect window)
{
	IplImage* sampleImg;//��������ͼ��
	CvPoint2D32f center;//������������
	CvHistogram* hist;//ֱ��ͼ
	
	int sizes[3] = {256, 256, 256};
	float ranges[3][2] = {{0,255}, {0,255}, {0,255}};
	float* rangesPtr = &ranges[0][0];

	//�õ���������ͼ��
	sampleImg = cvCreateImage(cvSize(window.width, window.height), img->depth, 3);
	center = cvPoint2D32f(window.x+window.width/2, window.y+window.height/2);
	cvGetRectSubPix(img, sampleImg, center);

	//����������ֵ�ֱ��ͼ
	hist = cvCreateHist(3, sizes, CV_HIST_ARRAY, &rangesPtr, 1);
	cvCalcHist(&sampleImg, hist);
	
	return hist;
}

int gesDetectHand(IplImage * img)
{
	return img->width;
}