#include "gesrec.h"

void gesFindContours(IplImage* src, IplImage* dst)
{
	CvMemStorage* storage;//动态存储单元
	CvSeq* contour = 0;//轮廓
	IplImage* gray;

	//创建源图像对应的灰度图像
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//通过灰度图像得到边缘
	cvSmooth(gray, dst, CV_BLUR, 3, 3, 0);
	cvNot(gray, dst);
	cvCanny(gray, dst, 120, 240, 3);
	//cvSobel(gray, dst, 2, 2, 3);

	//判断原点位置以确定是否需要反转图像
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	storage = cvCreateMemStorage(0);

	//释放内存
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&gray);
}