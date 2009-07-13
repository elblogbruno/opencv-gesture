#include "gesrec.h"

void gesFindContours(IplImage* src, IplImage* dst)
{
	CvMemStorage* storage;//��̬�洢��Ԫ
	CvSeq* contour = 0;//����
	IplImage* gray;

	//����Դͼ���Ӧ�ĻҶ�ͼ��
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);
	
	//ͨ���Ҷ�ͼ��õ���Ե
	cvSmooth(gray, dst, CV_BLUR, 3, 3, 0);
	cvNot(gray, dst);
	cvCanny(gray, dst, 120, 240, 3);
	//cvSobel(gray, dst, 2, 2, 3);

	//�ж�ԭ��λ����ȷ���Ƿ���Ҫ��תͼ��
	if(src->origin == 1)
	{
		cvFlip(dst);
	}

	storage = cvCreateMemStorage(0);

	//�ͷ��ڴ�
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&gray);
}