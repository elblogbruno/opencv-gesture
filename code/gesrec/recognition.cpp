#include "gesrec.h"

//轮廓面积比较函数
static int gesContourCompFunc(const void* _a, const void* _b, void* userdata)
{
	int retval;
	double s1, s2;
	CvContour* a = (CvContour*)_a;
	CvContour* b = (CvContour*)_b;
	
	s1 = fabs(cvContourArea(a));
	s2 = fabs(cvContourArea(b));
	//s1 = a->rect.height * a->rect.width;
	//s2 = b->rect.height * b->rect.width;

	if(s1 < s2)
	{
		retval = 1;
	}
	else if(s1 == s2)
	{
		retval = 0;
	}
	else
	{
		retval = -1;
	}

	return retval;
}

//src:BGR dst:
void gesFindContours(IplImage* src, IplImage* dst, CvMemStorage* storage, CvSeq* contour)
{
	int count;//轮廓数
	IplImage* gray;
	CvMemStorage* first_sto;
	CvSeq* first_cont;
	CvSeq* cur_cont;

	first_sto = cvCreateMemStorage(0);
	first_cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), first_sto);

	//创建源图像对应的灰度图像
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//得到图像的外层轮廓
	if(contour != 0)
	{
		cvClearSeq(contour);
	}
	count = cvFindContours(gray, first_sto, &first_cont, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//如果没有检测到轮廓则返回
	if(first_sto == NULL)
	{
		return;
	}

	//将所有的轮廓都放到all_cont中
	for(;first_cont != 0;first_cont = first_cont->h_next)
	{
		//if(((CvContour* )first_cont)->rect.height * ((CvContour* )first_cont)->rect.width >= 625)
			cvSeqPush(contour, first_cont);
	}

	//对轮廓按照面积进行排序
	cvSeqSort(contour, gesContourCompFunc, 0);

	//在dst中画出轮廓
	cvZero(dst);
	for(int i = 0;i < min(contour->total, 4);i++)///////////////////////次数待改
	{
		cur_cont = (CvSeq* )cvGetSeqElem(contour, i);
		CvScalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
		cvDrawContours(dst, cur_cont, color, color, -1, 1, 8);
	}

	//判断原点位置以确定是否需要反转图像
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	//释放内存
	cvReleaseMemStorage(&first_sto);
	cvReleaseImage(&gray);
}