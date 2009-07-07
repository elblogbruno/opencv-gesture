#include "gesrec.h"

CvHistogram* gesSampleSkin(const IplImage* src, CvRect window)
{
	IplImage* sampleImg;//��������ͼ��
	IplImage* sampleHSV;
	IplImage* sampleHue;
	CvPoint2D32f center;//������������
	CvHistogram* hist;//ֱ��ͼ
	
	int sizes[1] = {256};
	float ranges[1][2] = {{0,360}};
	float** rangesPtr = new float* [1];
	rangesPtr[0] = ranges[0];

	//�õ���������ͼ��
	sampleImg = cvCreateImage(cvSize(window.width, window.height), src->depth, 3);
	center = cvPoint2D32f(window.x+window.width/2, window.y+window.height/2);
	cvGetRectSubPix(src, sampleImg, center);
	
	//�õ�����ͼ���Hue����
	sampleHSV = cvCreateImage(cvGetSize(sampleImg), IPL_DEPTH_8U, 3);
	sampleHue = cvCreateImage(cvGetSize(sampleImg), IPL_DEPTH_8U, 1);
	cvCvtColor(sampleImg, sampleHSV, CV_BGR2HSV);
	cvSplit(sampleHSV, sampleHue, NULL, NULL, NULL);

	//����������ֵ�ֱ��ͼ
	hist = cvCreateHist(1, sizes, CV_HIST_ARRAY, rangesPtr, 1);
	cvCalcHist(&sampleHue, hist);
	
	//�ͷ��ڴ�
	cvReleaseImage(&sampleImg);
	cvReleaseImage(&sampleHSV);
	cvReleaseImage(&sampleHue);
	delete(rangesPtr);

	return hist;
}

int gesDetectHand(IplImage* src, IplImage* dst)
{
	int threshold1 = 120;
	int threshold2 = 30;
	int block_size = 1000;
	int level = 4;
	int i;
	
	CvConnectedComp* cur_comp;
	CvSeq* comp;
	CvMemStorage* storage;
	IplImage* srcHSV;
	IplImage* srcHue;

	srcHSV = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	srcHue = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, srcHSV, CV_BGR2HSV);
	cvSplit(srcHSV, srcHue, NULL, NULL, NULL);

	//��ͼ����н������ָ�
	storage = cvCreateMemStorage(block_size);
	cvPyrSegmentation(srcHue, dst, storage, &comp, level, threshold1, threshold2);
	i = 0;
	while(i < comp->total)
	{
		cur_comp = (CvConnectedComp* )cvGetSeqElem(comp, i);
		cvRectangle(dst, cvPoint(cur_comp->rect.x, cur_comp->rect.y),
			cvPoint(cur_comp->rect.x+cur_comp->rect.width, cur_comp->rect.y+cur_comp->rect.height),
			cvScalar(255, 0, 0), 1);
		i++;
	}
	
	cvReleaseImage(&srcHSV);
	cvReleaseImage(&srcHue);

	return comp->total;
}