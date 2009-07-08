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

int gesDetectHandHistogram(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window)
{
	IplImage* srcHSV;
	IplImage* srcHue;
	IplImage* curImg;//��ǰ����Ŀ�
	CvPoint center;//ÿһС�������
	CvHistogram* hist;//ֱ��ͼ
	int boundryX, boundryY;
	int stepX, stepY;

	int sizes[1] = {256};
	float ranges[1][2] = {{0,360}};
	float** rangesPtr = new float* [1];
	rangesPtr[0] = ranges[0];

	//���ȶ�Դͼ�����ɫ��ƽ��
	//gesGrayWorld(src, src);

	//�õ�Դͼ���Hue����
	srcHSV = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	srcHue = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, srcHSV, CV_BGR2HSV);
	cvSplit(srcHSV, srcHue, NULL, NULL, NULL);

	//��ͼ�����ֱ��ͼƥ��
	curImg = cvCreateImage(cvSize(window.width, window.height), IPL_DEPTH_8U, 1);
	center = cvPoint(window.width/2, window.height/2);
	hist = cvCreateHist(1, sizes, CV_HIST_ARRAY, rangesPtr, 1);
	stepX = window.width / 2;
	stepY = window.height / 2;
	boundryX = src->width - stepX;
	boundryY = src->height - stepY;
	cvNormalizeHist(histTemp, 1);
	for(;center.x < boundryX;center.x += window.width)
	{
		for(center.y = stepY;center.y < boundryY;center.y += window.height)
		{
			cvGetRectSubPix(srcHue, curImg, cvPointTo32f(center));
			cvCalcHist(&curImg, hist);
			cvNormalizeHist(hist, 1);
			//if(cvCompareHist(histTemp, hist, CV_COMP_CORREL) >= 0.4)
			//if(cvCompareHist(histTemp, hist, CV_COMP_CHISQR) <= 0.3)
			if(cvCompareHist(histTemp, hist, CV_COMP_INTERSECT) >= 0.1)
			{
				cvRectangle(dst, cvPoint(center.x - stepX, center.y - stepY),
					cvPoint(center.x + stepX, center.y + stepY), cvScalar(255, 0, 0), 1);
			}
		}
	}
	
	//�ͷ��ڴ�
	cvReleaseImage(&srcHSV);
	cvReleaseImage(&srcHue);
	cvReleaseImage(&curImg);
	cvReleaseHist(&hist);
	delete(rangesPtr);

	return 0;
}

void gesDetectHandRange(IplImage* src, IplImage* dst)
{
	IplImage* srcYCrCb;
	CvScalar s;
	int i,j;

	//��Դͼ��ת����YCrCb�ռ�
	srcYCrCb = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvCvtColor(src, srcYCrCb, CV_BGR2YCrCb);

	//����ͼ���ҵ����ڷ�ɫ��Χ����Щ����
	for(i = 0;i < srcYCrCb->width;i++)
	{
		for(j = 0;j < srcYCrCb->height;j++)
		{
			s = cvGet2D(srcYCrCb, j, i);
			if(s.val[1] >= 133 && s.val[1] <= 173 && s.val[2] >= 77 && s.val[2] <= 127)
			{
				s.val[0] = s.val[1] = s.val[2] = 255;
				cvSet2D(dst, j, i, s);
			}
		}
	}

	cvReleaseImage(&srcYCrCb);
}

void gesGrayWorld(IplImage* src, IplImage* dst)
{
	int i,j;
	double avgR, avgG, avgB, avgGray;
	double factor;
	double ar, ag, ab;
	CvScalar s;
	avgR = avgG = avgB = 0;

	for(i = 0;i < src->width;i++)
	{
		for(j = 0;j < src->height;j++)
		{
			s = cvGet2D(src, j, i);
			avgR += s.val[2];
			avgG += s.val[1];
			avgB += s.val[0];
		}
	}

	//����ƽ����r g bֵ
	avgR /= (src->width*src->height);
	avgG /= (src->width*src->height);
	avgB /= (src->width*src->height);
	avgGray = (avgR + avgG + avgB) / 3;
	ar = avgGray / avgR;
	ag = avgGray / avgG;
	ab = avgGray / avgB;

	//����r g bֵ����
	for(i = 0;i < src->width;i++)
	{
		for(j = 0;j < src->height;j++)
		{
			s = cvGet2D(src, j, i);
			s.val[2] = s.val[2] * ar;
			s.val[1] = s.val[1] * ag;
			s.val[0] = s.val[0] * ab;
			factor = max(s.val[0], max(s.val[1], s.val[2]));
			factor /= 255;
			if(factor > 1)
			{
				s.val[0] /= factor;
				s.val[1] /= factor;
				s.val[2] /= factor;
			}
			cvSet2D(dst, j, i, s);
		}
	}
}