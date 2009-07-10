#include "gesrec.h"

CvHistogram* gesSampleSkinHistogram(IplImage* src, CvRect window)
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

void gesDetectHandHistogram(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window)
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
}

void gesSampleSkinRange(IplImage* src, CvScalar* s)
{
	IplImage* srcYCrCb;
	CvScalar tempS;
	int i,j;

	//��Դͼ��ת����YCrCb�ռ�
	srcYCrCb = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvCvtColor(src, srcYCrCb, CV_BGR2YCrCb);

	//����ͼ�񣬼�����Ʒ��ƽ��Cr Cb����ֵ
	s->val[0] = s->val[1] = s->val[2] = 0;
	for(i = 0;i < srcYCrCb->width;i++)
	{
		for(j = 0;j < srcYCrCb->height;j++)
		{
			tempS = cvGet2D(srcYCrCb, j, i);
			s->val[0] += tempS.val[0];
			s->val[1] += tempS.val[1];
			s->val[2] += tempS.val[2];
		}
	}
	s->val[0] /= (src->width * src->height);
	s->val[1] /= (src->width * src->height);
	s->val[2] /= (src->width * src->height);

	//�ͷ��ڴ�
	cvReleaseImage(&srcYCrCb);
}

//dstΪ��ͨ����ͼ��
void gesDetectHandRange(IplImage* src, IplImage* dst, CvScalar* s, int flag)
{
	IplImage* srcYCrCb;
	CvScalar tempS;
	//IplConvKernel* kernel;
	int i,j;
	double range1, range2, range3, range4;

	//gesGrayWorld(src, src);
	cvSmooth(src, src, CV_GAUSSIAN);

	//��Դͼ��ת����YCrCb�ռ�
	srcYCrCb = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvCvtColor(src, srcYCrCb, CV_BGR2YCrCb);

	//���÷�ɫ�ķ�Χ
	if(flag)//������Ʒ���÷�ɫ��Χ
	{
		range1 = s->val[1] - 10;
		range2 = s->val[1] + 10;
		range3 = s->val[2] - 10;
		range4 = s->val[2] + 10;
	}
	else//�Զ����÷�ɫ��Χ
	{
		range1 = 133;
		range2 = 173;
		range3 = 77;
		range4 = 127;
	}

	//����ͼ���ҵ����ڷ�ɫ��Χ����Щ����
	for(i = 0;i < srcYCrCb->width;i++)
	{
		for(j = 0;j < srcYCrCb->height;j++)
		{
			tempS = cvGet2D(srcYCrCb, j, i);
			if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && tempS.val[2] >= range3 && tempS.val[2] <= range4)
			{
				tempS.val[0] = tempS.val[1] = tempS.val[2] = 255;
			}
			else
			{
				tempS.val[0] = tempS.val[1] = tempS.val[2] = 0;
			}
			cvSet2D(dst, j, i, tempS);
		}
	}

	gesMultiFloodFill(dst);

	//�Խ��ͼ����п�������ȥ������
	//kernel = cvCreateStructuringElementEx(7, 7, 3, 3, CV_SHAPE_RECT);
	//cvMorphologyEx(dst, dst, NULL, kernel, CV_MOP_OPEN);

	//�ͷ��ڴ�
	cvReleaseImage(&srcYCrCb);
	//cvReleaseStructuringElement(&kernel);
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

//��ͨ����������򷽷�
static int gesRectCompFunc(const void* _a, const void* _b, void* userdata)
{
	int retval;
	CvConnectedComp* a = (CvConnectedComp*)_a;
	CvConnectedComp* b = (CvConnectedComp*)_b;

	if(a->area < b->area)
	{
		retval = 1;
	}
	else if(a->area == b->area)
	{
		retval = 0;
	}
	else
	{
		retval = -1;
	}

	return retval;
}

CvSeq* gesMultiFloodFill(IplImage* src)
{
	IplImage* mask;//��������
	CvScalar tempS1, tempS2;
	CvConnectedComp tempComp;
	CvSeq* comp;//��ͨ����
	CvMemStorage* storage;//��̬�ڴ�
	CvConnectedComp* cur_comp;
	int i, j;
	int l_comp;

	//��ʼ����̬�ڴ�����ͨ����
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);

	//��ʼ������
	mask = cvCreateImage(cvSize(src->width + 2, src->height + 2), IPL_DEPTH_8U, 1);
	cvZero(mask);

	//����ͼ��Ѱ����ͨ����
	for(i = 0;i < src->width;i++)
	{
		for(j = 0;j < src->height;j++)
		{
			tempS1 = cvGet2D(src, j, i);
			tempS2 = cvGet2D(mask, j, i);
			//�жϸ������Ƿ������
			if(tempS1.val[0] != 0 && tempS2.val[0] == 0)
			{
				cvFloodFill(src, cvPoint(i, j), cvScalar(255), cvScalarAll(0), 
					cvScalarAll(0), &tempComp, 8 | CV_FLOODFILL_MASK_ONLY, mask);
				if(tempComp.area >= 100)//�޳���Щ�����С����ͨ����(����������)
				{
					cvSeqPush(comp, &tempComp);
				}
			}
		}
	}

	//�þ��λ�����ͨ����
	i = 0;
	l_comp = comp->total;
	cvSeqSort(comp, gesRectCompFunc, 0);//����ͨ�����������
	while(i < min(l_comp, 4))
	{
		cur_comp = (CvConnectedComp* )cvGetSeqElem(comp, i);
		cvRectangle(src, cvPoint(cur_comp->rect.x, cur_comp->rect.y),
					cvPoint(cur_comp->rect.x + cur_comp->rect.width, cur_comp->rect.y + cur_comp->rect.height), 
					cvScalar(255, 0, 0), 1);
		i++;
	}

	//�ͷ��ڴ�
	cvReleaseMemStorage(&storage);//ò�Ʋ����������ͷţ���Ϊ��Ҫ��
	cvReleaseImage(&mask);

	return comp;
}