#include "gesrec.h"

void gesFindContours(IplImage* src, IplImage* dst)
{
	CvMemStorage* storage;//动态存储单元
	CvSeq* contour = 0;//轮廓

	cvCvtColor(src, dst, CV_BGR2GRAY);
	//cvThreshold(dst, dst, 90, 255, CV_THRESH_BINARY);
	cvCanny(dst, dst, 100, 100, 3);

	storage = cvCreateMemStorage(0);

	//释放内存
	cvReleaseMemStorage(&storage);
}