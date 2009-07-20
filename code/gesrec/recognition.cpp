#include "gesrec.h"
#include <stdio.h>//////////////////////////////////////////

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
void gesFindContours(IplImage* src, IplImage* dst, CvSeq** templateContour, CvMemStorage* templateStorage, int flag)
{
	int count;//轮廓数
	IplImage* gray;
	CvMemStorage* first_sto;
	CvMemStorage* all_sto;
	CvSeq* first_cont;
	CvSeq* all_cont;
	CvSeq* cur_cont;
	
	//初始化动态内存
	first_sto = cvCreateMemStorage(0);
	first_cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), first_sto);
	all_sto = cvCreateMemStorage(0);
	all_cont = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), all_sto);

	//创建源图像对应的灰度图像
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//得到图像的外层轮廓
	count = cvFindContours(gray, first_sto, &first_cont, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//如果没有检测到轮廓则返回
	if(first_sto == NULL)
	{
		return;
	}

	//将所有的轮廓都放到first_cont中
	for(;first_cont != 0;first_cont = first_cont->h_next)
	{
		if(((CvContour* )first_cont)->rect.height * ((CvContour* )first_cont)->rect.width >= 625)
			cvSeqPush(all_cont, first_cont);
	}

	//对轮廓按照面积进行排序
	cvSeqSort(all_cont, gesContourCompFunc, 0);

	//在dst中画出轮廓
	cvZero(dst);
	for(int i = 0;i < min(all_cont->total, 3);i++)///////////////////////次数待改
	{
		cur_cont = (CvSeq* )cvGetSeqElem(all_cont, i);
		if(flag != 0 && i == 0)
		{
			*templateContour = cvCloneSeq(cur_cont, templateStorage);
		}
		
		CvScalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
		cvDrawContours(dst, (CvSeq* )cur_cont, color, color, -1, 1, 8);
	}
	
	//判断原点位置以确定是否需要反转图像
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	//释放内存
	cvReleaseMemStorage(&first_sto);
	cvReleaseMemStorage(&all_sto);
	cvReleaseImage(&gray);
}

void gesMatchContoursTemplate(IplImage* src, IplImage* dst, CvSeq** templateContour)
{
	CvSeq* contour;
	CvMemStorage* storage;

	//初始化动态内存
	storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), storage);

	//得到轮廓并进行匹配
	gesFindContours(src, dst, &contour, storage, 1);
	if(contour->total != 0)//如果得到的轮廓不为空
	{
		double result = cvMatchShapes((CvContour* )contour, (CvContour* )(*templateContour), CV_CONTOURS_MATCH_I3);
		printf("%.2f\n", result);/////////////////////////////////////////////
	}

	//释放内存
	cvReleaseMemStorage(&storage);
}

//找出轮廓最大的5个极大值点
void gesFindContourMaxs(CvSeq* contour)
{
	int i;
	CvScalar center;//重心位置
	CvPoint* p;
	CvMat max;//存储5个极大值的数组
	double initMax[] = {-1, -1, -1, -1, -1};//初始极大值设置为-1
	double minValue, maxValue;//5个极大值中的最大值与最小值
	CvPoint minLoc;//最小值的位置
	double preDistance = 0;
	bool isCandidate = false;//是否是候选的极大值点

	//初始化重心位置
	center = cvScalarAll(0);

	//初始化极大值矩阵
	max = cvMat(1, 5, CV_64FC1, initMax);

	//首先求出轮廓的重心
	for(i = 0;i < contour->total;i++)
	{
		p = (CvPoint* )cvGetSeqElem(contour, i);
		center.val[0] += p->x;
		center.val[1] += p->y;
	}
	center.val[0] /= contour->total;
	center.val[1] /= contour->total;

	//遍历轮廓,找出所有的极大值点
	for(i = 0;i < contour->total;i++)
	{
		p = (CvPoint* )cvGetSeqElem(contour, i);
		double distance = sqrt(pow(center.val[0] - p->x, 2) + pow(center.val[1] - p->y, 2));

		if(distance > preDistance)
		{
			isCandidate = true;
		}
		else if(distance < preDistance && isCandidate == true)
		{
			cvMinMaxLoc(&max, &minValue, &maxValue, &minLoc);

			if(distance > minValue)
			{
				cvmSet(&max, minLoc.y, minLoc.x, distance);
			}
			isCandidate = false;
		}
		else
		{
			isCandidate = false;
		}

		preDistance = distance;
	}

	//打印5个极大值
	printf("%.2f %.2f %.2f %.2f %.2f\n", cvmGet(&max, 0, 0), cvmGet(&max, 0, 1), cvmGet(&max, 0, 2), cvmGet(&max, 0, 3), cvmGet(&max, 0, 4));
}

//计算轮廓的pair-wise几何直方图并进行匹配
CvHistogram* gesMatchContoursPGH(CvSeq* contour)
{
	//CvHistogram* hist;

	//cvCalcPGH(contour, hist);

	return NULL;
}