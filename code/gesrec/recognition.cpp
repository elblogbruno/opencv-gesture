#include "gesrec.h"

//src:BGR dst:
void gesFindContours(IplImage* src, IplImage* dst, CvMemStorage* storage, CvSeq* contour)
{
	IplImage* gray;

	//创建源图像对应的灰度图像
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//得到图像的外层轮廓
	if(contour != 0)
	{
		cvClearSeq(contour);
	}
	cvFindContours(gray, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//在dst中画出轮廓
	cvZero(dst);
	for(;contour != 0;contour = contour->h_next)
	{
		CvScalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
		cvDrawContours(dst, contour, color, color, -1, 1, 8);
	}

	//判断原点位置以确定是否需要反转图像
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	//释放内存
	cvReleaseImage(&gray);
}