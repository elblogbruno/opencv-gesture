#include "gesrec.h"

//src:BGR dst:
void gesFindContours(IplImage* src, IplImage* dst, CvMemStorage* storage, CvSeq* contour)
{
	IplImage* gray;

	//����Դͼ���Ӧ�ĻҶ�ͼ��
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//�õ�ͼ����������
	if(contour != 0)
	{
		cvClearSeq(contour);
	}
	cvFindContours(gray, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//��dst�л�������
	cvZero(dst);
	for(;contour != 0;contour = contour->h_next)
	{
		CvScalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
		cvDrawContours(dst, contour, color, color, -1, 1, 8);
	}

	//�ж�ԭ��λ����ȷ���Ƿ���Ҫ��תͼ��
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	//�ͷ��ڴ�
	cvReleaseImage(&gray);
}