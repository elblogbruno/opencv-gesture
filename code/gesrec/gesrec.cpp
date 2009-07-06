#include "gesrec.h"

CvHistogram* gesSampleSkin(const IplImage* img, CvRect window)
{
	IplImage* sampleImg;//采样部分图像
	CvPoint2D32f center;//采样部分中心
	CvHistogram* hist;//直方图
	
	int sizes[3] = {256, 256, 256};
	float ranges[3][2] = {{0,255}, {0,255}, {0,255}};
	float* rangesPtr = &ranges[0][0];

	//得到采样部分图像
	sampleImg = cvCreateImage(cvSize(window.width, window.height), img->depth, 3);
	center = cvPoint2D32f(window.x+window.width/2, window.y+window.height/2);
	cvGetRectSubPix(img, sampleImg, center);

	//计算采样部分的直方图
	hist = cvCreateHist(3, sizes, CV_HIST_ARRAY, &rangesPtr, 1);
	cvCalcHist(&sampleImg, hist);
	
	return hist;
}

int gesDetectHand(IplImage * img)
{
	return img->width;
}