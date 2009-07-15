#include "gesrec.h"

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
void gesFindContours(IplImage* src, IplImage* dst, CvMemStorage* storage, CvSeq* contour)
{
	int count;//������
	IplImage* gray;
	CvMemStorage* first_sto;
	CvSeq* first_cont;
	CvSeq* cur_cont;

	first_sto = cvCreateMemStorage(0);
	first_cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), first_sto);

	//����Դͼ���Ӧ�ĻҶ�ͼ��
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//�õ�ͼ����������
	if(contour != 0)
	{
		cvClearSeq(contour);
	}
	count = cvFindContours(gray, first_sto, &first_cont, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//���û�м�⵽�����򷵻�
	if(first_sto == NULL)
	{
		return;
	}

	//�����е��������ŵ�all_cont��
	for(;first_cont != 0;first_cont = first_cont->h_next)
	{
		//if(((CvContour* )first_cont)->rect.height * ((CvContour* )first_cont)->rect.width >= 625)
			cvSeqPush(contour, first_cont);
	}

	//���������������������
	cvSeqSort(contour, gesContourCompFunc, 0);

	//��dst�л�������
	cvZero(dst);
	for(int i = 0;i < min(contour->total, 4);i++)///////////////////////��������
	{
		cur_cont = (CvSeq* )cvGetSeqElem(contour, i);
		CvScalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
		cvDrawContours(dst, cur_cont, color, color, -1, 1, 8);
	}

	//�ж�ԭ��λ����ȷ���Ƿ���Ҫ��תͼ��
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	//�ͷ��ڴ�
	cvReleaseMemStorage(&first_sto);
	cvReleaseImage(&gray);
}