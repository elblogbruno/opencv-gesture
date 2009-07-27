#include "gesrec.h"
#include <stdio.h>//////////////////////////////////////////

#define PI 3.14159f

//��������ȽϺ���
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
	int count;//������
	IplImage* gray;
	CvMemStorage* first_sto;
	CvMemStorage* all_sto;
	CvSeq* first_cont;
	CvSeq* all_cont;
	CvSeq* cur_cont;
	
	//��ʼ����̬�ڴ�
	first_sto = cvCreateMemStorage(0);
	first_cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), first_sto);
	all_sto = cvCreateMemStorage(0);
	all_cont = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), all_sto);

	//����Դͼ���Ӧ�ĻҶ�ͼ��
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//�õ�ͼ����������
	count = cvFindContours(gray, first_sto, &first_cont, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//���û�м�⵽�����򷵻�
	if(first_sto == NULL)
	{
		return;
	}

	//�����е��������ŵ�first_cont��
	for(;first_cont != 0;first_cont = first_cont->h_next)
	{
		if(((CvContour* )first_cont)->rect.height * ((CvContour* )first_cont)->rect.width >= 625)
			cvSeqPush(all_cont, first_cont);
	}

	//���������������������
	cvSeqSort(all_cont, gesContourCompFunc, 0);

	//��dst�л�������
	cvZero(dst);
	for(int i = 0;i < min(all_cont->total, 3);i++)///////////////////////��������
	{
		cur_cont = (CvSeq* )cvGetSeqElem(all_cont, i);
		if(flag != 0 && i == 0)
		{
			*templateContour = cvCloneSeq(cur_cont, templateStorage);
		}

		CvScalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
		cvDrawContours(dst, (CvSeq* )cur_cont, color, color, -1, 1, 8);
	}
	
	//�ж�ԭ��λ����ȷ���Ƿ���Ҫ��תͼ��
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	//�ͷ��ڴ�
	cvReleaseMemStorage(&first_sto);
	cvReleaseMemStorage(&all_sto);
	cvReleaseImage(&gray);
}

void gesMatchContoursTemplate(IplImage* src, IplImage* dst, CvSeq** templateContour)
{
	CvSeq* contour;
	CvMemStorage* storage;

	//��ʼ����̬�ڴ�
	storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), storage);

	//�õ�����������ƥ��
	gesFindContours(src, dst, &contour, storage, 1);
	if(contour->total != 0)//����õ���������Ϊ��
	{
		double result = cvMatchShapes((CvContour* )contour, (CvContour* )(*templateContour), CV_CONTOURS_MATCH_I3);
		printf("%.2f\n", result);/////////////////////////////////////////////
	}

	//�ͷ��ڴ�
	cvReleaseMemStorage(&storage);
}

//ģ��ƥ�䷨������ʵ��
void gesMatchContoursTemplate2(IplImage* src, IplImage* dst, CvSeq* templateContour)
{
	CvSeq* contour;
	CvSeq* cur_cont;
	CvMemStorage* storage;
	double minValue, minIndex, tempValue;
	int i;

	//��ʼ����̬�ڴ�
	storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), storage);

	printf("aaaaa\n");

	//�õ�����������ƥ��
	minIndex = -1;
	gesFindContours(src, dst, &contour, storage, 1);
	if(contour->total != 0)//����õ���������Ϊ��
	{
		if(templateContour->total != 0)
		{
			cur_cont = (CvSeq* )cvGetSeqElem(templateContour, 0);
			minValue = cvMatchShapes((CvContour* )contour, (CvContour* )cur_cont, CV_CONTOURS_MATCH_I3);
			minIndex = 0;
		}
		printf("bbbbb\n");
		for(i = 1;i < templateContour->total;i++)
		{
			cur_cont = (CvSeq* )cvGetSeqElem(templateContour, i);
			tempValue = cvMatchShapes((CvContour* )contour, (CvContour* )cur_cont, CV_CONTOURS_MATCH_I3);
			if(tempValue < minValue)
			{
				minValue = tempValue;
				minIndex = i;
			}
		}
	}

	//��ӡƥ����
	printf("the result is %d\n", minIndex);

	//�ͷ��ڴ�
	cvReleaseMemStorage(&storage);
}

//�ҳ���������5������ֵ��
void gesFindContourMaxs(CvSeq* contour)
{
	int i;
	CvScalar center;//����λ��
	CvPoint* p;
	CvMat max;//�洢5������ֵ������
	double initMax[] = {-1, -1, -1, -1, -1};//��ʼ����ֵ����Ϊ-1
	double minValue, maxValue;//5������ֵ�е����ֵ����Сֵ
	CvPoint minLoc;//��Сֵ��λ��
	double preDistance = 0;
	bool isCandidate = false;//�Ƿ��Ǻ�ѡ�ļ���ֵ��

	//��ʼ������λ��
	center = cvScalarAll(0);

	//��ʼ������ֵ����
	max = cvMat(1, 5, CV_64FC1, initMax);

	//�����������������
	for(i = 0;i < contour->total;i++)
	{
		p = (CvPoint* )cvGetSeqElem(contour, i);
		center.val[0] += p->x;
		center.val[1] += p->y;
	}
	center.val[0] /= contour->total;
	center.val[1] /= contour->total;

	//��������,�ҳ����еļ���ֵ��
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

	//��ӡ5������ֵ
	printf("%.2f %.2f %.2f %.2f %.2f\n", cvmGet(&max, 0, 0), cvmGet(&max, 0, 1), cvmGet(&max, 0, 2), cvmGet(&max, 0, 3), cvmGet(&max, 0, 4));
}

//����������pair-wise����ֱ��ͼ
CvHistogram* gesCalcContoursPGH(CvSeq* contour)
{
	CvHistogram* hist;//�ɶԼ���ֱ��ͼ
	CvContour* tempCont;
	
	//�õ��ɶԼ���ֱ��ͼ�ڶ���ά���ϵķ�Χ
	tempCont = (CvContour* )contour;
	cvBoundingRect(tempCont, 1);

	int sizes[2] = {60, 200};
	float ranges[2][2] = {{0,PI}, {0,200}};
	float** rangesPtr = new float* [2];
	rangesPtr[0] = ranges[0];
	rangesPtr[1] = ranges[1];

	//��ʼ������ֱ��ͼ
	hist = cvCreateHist(2, sizes, CV_HIST_ARRAY, rangesPtr, 1);

	//���������ĳɶԼ���ֱ��ͼ
	cvCalcPGH(contour, hist);

	return hist;
}

//��������pair-wise����ֱ��ͼ����ƥ��
void gesMatchContoursPGH(CvSeq* contour, CvHistogram* templateHist)
{
	CvHistogram* hist;

	//�õ������ĳɶԼ���ֱ��ͼ
	hist = gesCalcContoursPGH(contour);

	//��һ��ֱ��ͼ
	cvNormalizeHist(templateHist, 1);
	cvNormalizeHist(hist, 1);

	//ֱ��ͼƥ��
	double result = cvCompareHist(hist, templateHist, CV_COMP_INTERSECT);
	printf("result:%.2f\n", result);

	//�ͷ��ڴ�
	cvReleaseHist(&hist);
}