// gesrecTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

char output[40];

void histogram(IplImage* src, IplImage* dst);
int testSampleHistogram(int argc, char** argv);
int testImgDetectHandRange(int argc, char** argv);
int testCamDetectHandRange(int argc, char** argv);
int getImgFromCAM(int argc, char** argv);
int testMatchTemplate(int argc, char** argv);
IplImage secaipingheng( IplImage* seping);
int testSecaipingheng(int argc, char** argv);
int testMaxs(int argc, char** argv);
int testPGH(int argc, char** argv);
int testPGHImg(int argc, char** argv);
int testMatchTemplate2(int argc, char** argv);

//显示输入图像在HSV颜色空间的H分量的直方图
void histogram(IplImage* src, IplImage* dst)
{
	CvHistogram* hist;
	int bin_w;
	float max_val;

	hist = gesSampleSkinHistogram(src, cvRect(0,0,src->width,src->height));
	cvZero(dst);
	cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0);
	cvConvertScale(hist->bins, hist->bins, max_val?255./max_val:0., 0);
	bin_w = dst->width/256;

	for(int i = 0;i < 256;i++)
	{
		double val = (cvGetReal1D(hist->bins, i)*dst->height/255);
		CvScalar color = CV_RGB(255, 255, 0);
		cvRectangle(dst, cvPoint(i*bin_w,dst->height), cvPoint((i+1)*bin_w,(int)(dst->height-val)), color, 1, 8, 0);
	}

	cvReleaseHist(&hist);
}

//调用histogram方法的一个具体测试函数
//cmd使用说明:gesrecTest.exe [index = 0] [filename]
int testSampleHistogram(int argc, char** argv)
{
	IplImage* input;//样本图片
	IplImage* output;//直方图片

	if(argc == 3 && 
		(input = cvLoadImage(argv[2], 1)) != 0)
	{
		cvNamedWindow("Input", 1);
		cvNamedWindow("Output", 1);

		output = cvCreateImage(cvSize(320, 200), IPL_DEPTH_8U, 3);
		histogram(input, output);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvWaitKey(0);

		//释放内存
		cvDestroyWindow("Input");
		cvDestroyWindow("Output");
		cvReleaseImage(&input);
		cvReleaseImage(&output);

		return 1;
	}
	return 0;
}

//检测静态图片中的皮肤
//cmd使用说明:gesrecTest.exe [index = 1] [filename:src] ([filename:sample])
int testImgDetectHandRange(int argc, char** argv)
{
	IplImage* sampleImg;//样本图片
	IplImage* input;//要检测的图片
	IplImage* output;//输出的结果图片
	IplImage* gray;//////////////////////////////////////////////////////////////
	IplImage* tempCorner;///////////////////////////////////////////////////////
	IplImage* corner;/////////////////////////////////////////////////////////////角点
	CvPoint2D32f corners[100];//////////////////////////////////////////////////
	int cornerCount = 100;//////////////////////////////////////////////////
	CvScalar s;//样本直方图
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	int i;///////////////////////////////////////////////////////////////////////////

	//载入图像
	if( argc >= 3 && 
		(input = cvLoadImage( argv[2], 1)) != 0)
	{
		//初始化动态内存与连通部件
		storage = cvCreateMemStorage(0);
		comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);

		//获得样本图片的肤色范围
		if(argc == 4)
		{
			if((sampleImg = cvLoadImage(argv[3], 1)) != 0)
			{
				gesSampleSkinRange(sampleImg, &s);
				cvReleaseImage(&sampleImg);
			}
			else
			{
				return 0;
			}
		}

		//生成输出图片
		output = cvCloneImage(input);
		gray = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);/////////////////////
		tempCorner = cvCreateImage(cvGetSize(input), IPL_DEPTH_32F, 1);//////////////////////
		corner = cvCreateImage(cvGetSize(input), IPL_DEPTH_32F, 1);//////////////////////
		if(argc == 4)
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}
		else
		{
			gesDetectHandRange(input, output, comp);
		}

		cvCvtColor(output, gray, CV_BGR2GRAY);///////////////////////////////////
		//cvPreCornerDetect(gray, corner, 3);///////////////////////////////////////////
		//cvCornerMinEigenVal(gray, corner, 3, 3);/////////////////////////////
		cvGoodFeaturesToTrack(gray, corner, tempCorner, corners, &cornerCount, 0.2, 50, 0, 3, 0, 0.4);///////////////////////
		printf("num corners found: %d\n", cornerCount);///////////////////////////

		if(cornerCount > 0)
		{
			for(i = 0;i < cornerCount;i++)
			{
				cvCircle(input, cvPoint((int)(corners[i].x), (int)(corners[i].y)), 6, CV_RGB(255, 0 ,0) , 2, CV_AA, 0);
			}
		}

		cvNamedWindow("Input", 1);
		cvShowImage("Input", input);
		cvNamedWindow("Output", 1);
		cvShowImage("Output", output);
		cvNamedWindow("Corner", 1);//////////////////////////////////////////////////////
		cvShowImage("Corner", corner);///////////////////////////////////////////////////

		cvWaitKey(0); //等待按键

		cvReleaseMemStorage(&storage);
		cvDestroyWindow("Input");
		cvDestroyWindow("Output");
		cvDestroyWindow("Corner");/////////////////////////////////////////////////////////
		cvReleaseImage(&output);
		cvReleaseImage(&input);
		cvReleaseImage(&gray);///////////////////////////////////////////////////////
		cvReleaseImage(&tempCorner);/////////////////////////////////////////////////
		cvReleaseImage(&corner);/////////////////////////////////////////////////////
		return 1;
	}

	return 0;
}

//检测摄像头中的皮肤
//cmd使用说明:gesrecTest.exe [index = 2]
int testCamDetectHandRange(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvSeq* contour = 0;//轮廓信息
	CvMemStorage* conSto;//轮廓内存

	if(argc == 3)
	{
		if((sampleImg = cvLoadImage(argv[2], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("Contour", 1);

	//初始化动态内存与连通部件
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	conSto = cvCreateMemStorage(0);
	contour = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), conSto);

	//获得第一帧
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);
		if(argc == 2)
		{
			gesDetectHandRange(input, output, comp);
		}
		else
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}

		gesFindContours(output, conImg);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);
		if(cvWaitKey(10) >= 0)
		{
			cvSaveImage("Contour1.jpg", conImg);
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&conSto);
	cvReleaseImage(&output);
	cvReleaseImage(&conImg);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

//从摄像头中截图
//cmd使用说明:gesrecTest.exe [index = 3] [filename]
int getImgFromCAM(int argc, char** argv)
{
	CvCapture* capture = 0;

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return 0;
	}

	cvNamedWindow("Input", 1);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		IplImage* input = 0;

		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvShowImage("Input", input);
		if(cvWaitKey(10) >= 0)
		{
			if(!cvSaveImage(argv[2], input))
			{
				printf("Could not save : %s\n", argv[2]);
			}
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("Input");

	return 1;
}

//模版匹配法的测试,色彩平衡在目前的测试中没采用(因为看不出有什么效果)
//cmd使用说明:gesrecTest.exe [index = 4] ([filename:sample])
int testMatchTemplate(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;
	//IplImage* balance = 0;//色彩平衡后的图片
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvMemStorage* templateSto;
	CvSeq* templateContour;
	int isTemp = 0;
	int matching = 0;

	if(argc == 3)
	{
		if((sampleImg = cvLoadImage(argv[2], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("Contour", 1);

	//初始化动态内存与连通部件
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	templateSto = cvCreateMemStorage(0);
	templateContour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), templateSto);

	//获得第一帧
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		//得到色彩平衡后的图片
		//if(balance != 0)
		//{
		//	cvReleaseImage(&balance);
		//}
		//*balance = secaipingheng(input);

		cvReleaseImage(&output);
		//output = cvCloneImage(balance);
		output = cvCloneImage(input);
		if(argc == 2)
		{
			//gesDetectHandRange(balance, output, comp);
			gesDetectHandRange(input, output, comp);
		}
		else
		{
			//gesDetectHandRange(balance, output, comp, &s, 1);
			gesDetectHandRange(input, output, comp, &s, 1);
		}

		if(!matching)
		{
			gesFindContours(output, conImg, &templateContour, templateSto, isTemp);
		}else
		{
			gesMatchContoursTemplate(output, conImg, &templateContour);
		}
		if(isTemp == 1)
		{
			cvSaveImage("contour1.jpg", conImg);
			isTemp = 0;
			matching = 1;
		}

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);
		if(cvWaitKey(10) >= 20)
		{
			isTemp = 1;
		}
		else 
			if(cvWaitKey(10) >= 0)
			{
				break;
			}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&storage);
	cvReleaseMemStorage(&templateSto);
	cvReleaseImage(&output);
	cvReleaseImage(&conImg);
	//cvReleaseImage(&balance);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

//网上找的色彩平衡的算法
IplImage secaipingheng( IplImage* seping)
{
	IplImage* dst;//色彩平衡
	IplImage* gray;//gray图
	dst=cvCreateImage(cvSize(seping->width,seping->height),IPL_DEPTH_8U,3);
	gray=cvCreateImage(cvSize(seping->width,seping->height),IPL_DEPTH_8U,1);
	int height,width,step,channels;
	uchar *data,*data1;
	height = seping->height;
	width = seping->width;
	step = seping->widthStep;
	channels = seping->nChannels;
	data = (uchar *)seping->imageData;
	data1 = (uchar *)dst->imageData;
	int i,j;
	double R,G,B,Gy,aR,aG,aB;
	cvZero(dst);
	CvMat* MR=cvCreateMat(height,width,CV_64FC1);
	CvMat* MG=cvCreateMat(height,width,CV_64FC1);
	CvMat* MB=cvCreateMat(height,width,CV_64FC1);
	for(i=0;i<height;i++) for(j=0;j<width;j++)
	{
		R=data[i*step+j*channels+0];
		G=data[i*step+j*channels+1];
		B=data[i*step+j*channels+2];
		Gy=((gray->imageData + gray->widthStep*i))[j];
		cvmSet(MR,i,j,R);
		cvmSet(MG,i,j,G);
		cvmSet(MB,i,j,B);
		((gray->imageData + gray->widthStep*i))[j]=(char)(R+G+B)/3;
	}
	CvScalar argR,argG,argB;
	double argI;
	argR=cvAvg(MR,0);
	argG=cvAvg(MG,0);
	argB=cvAvg(MB,0);
	argI=(argR.val[0]+argG.val[0]+argB.val[0])/3;
	aR=argI/argR.val[0];
	aG=argI/argG.val[0];
	aB=argI/argB.val[0];
	for(i=0;i<height;i++) for(j=0;j<width;j++)
	{
		R=CV_MAT_ELEM(*MR,double,i,j)*aR;
		G=CV_MAT_ELEM(*MG,double,i,j)*aG;
		B=CV_MAT_ELEM(*MB,double,i,j)*aB;
		if(R>255)data1[i*step+j*channels+0]=255;
		else data1[i*step+j*channels+0]=(char)R;
		if(G>255)data1[i*step+j*channels+1]=255;
		else data1[i*step+j*channels+1]=(char)G;
		if(B>255)data1[i*step+j*channels+2]=255;
		else data1[i*step+j*channels+2]=(char)B;
	}
	cvNamedWindow("色彩平衡", CV_WINDOW_AUTOSIZE);
	cvShowImage("色彩平衡", dst);
	//cvWaitKey(0);
	//cvDestroyWindow( "色彩平衡" );//销毁窗口
	//cvReleaseImage( &dst ); //释放图像
	return *dst;
}

//色彩平衡算法的测试
//cmd使用说明:gesrecTest.exe [index = 5] [filename]
int testSecaipingheng(int argc, char** argv)
{
	IplImage* input;
	IplImage* output;

	if((input = cvLoadImage(argv[2], 1)) == 0)
	{
		printf("Could not load image\n");
		return 0;
	}
	output = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);
	*output = secaipingheng(input);

	cvNamedWindow("input", 1);
	cvShowImage("input", input);

	cvWaitKey(0);

	cvDestroyWindow("input");
	cvDestroyWindow("output");
	cvReleaseImage(&input);
	cvReleaseImage(&output);

	return 1;
}

//极值法的测试
//cmd使用说明:gesrecTest.exe [index = 6]
int testMaxs(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvSeq* contour;
	CvMemStorage* contSto;
	
	if(argc == 3)
	{
		if((sampleImg = cvLoadImage(argv[2], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("Contour", 1);

	//初始化动态内存
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	contSto = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), contSto);

	//获得第一帧
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);
		if(argc == 2)
		{
			gesDetectHandRange(input, output, comp);
		}
		else
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}

		gesFindContours(output, conImg, &contour, contSto, 1);
		gesFindContourMaxs(contour);

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);
		if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&contSto);
	cvReleaseImage(&output);
	cvReleaseImage(&conImg);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

//PGH法的测试
//cmd使用说明:gesrecTest.exe [index = 7]
int testPGH(int argc, char** argv)
{
	CvCapture* capture = 0;
	IplImage* input = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvSeq* contour;
	CvMemStorage* contSto;
	CvHistogram* hist = 0;
	int isTemp = 0;
	int matching = 0;
	
	if(argc == 3)
	{
		if((sampleImg = cvLoadImage(argv[2], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("Contour", 1);

	//初始化动态内存
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	contSto = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), contSto);

	//获得第一帧
	input = cvQueryFrame(capture);
	if(!input)
	{
		return 0;
	}
	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//循环捕捉,直到用户按键跳出循环体
	for( ; ; )
	{
		input = cvQueryFrame(capture);
		if(!input)
		{
			break;
		}

		cvReleaseImage(&output);
		output = cvCloneImage(input);
		if(argc == 2)
		{
			gesDetectHandRange(input, output, comp);
		}
		else
		{
			gesDetectHandRange(input, output, comp, &s, 1);
		}

		gesFindContours(output, conImg, &contour, contSto, 1);

		if(isTemp == 1)
		{
			isTemp = 0;
			matching = 1;
			if(hist != 0)
			{
				cvReleaseHist(&hist);
			}
			hist = gesCalcContoursPGH(contour);
		}

		if(matching == 1)
		{
			gesMatchContoursPGH(contour, hist);
		}

		cvShowImage("Input", input);
		cvShowImage("Output", output);
		cvShowImage("Contour", conImg);
		if(cvWaitKey(10) >= 20)
		{
			isTemp = 1;
		}
		else if(cvWaitKey(10) >= 0)
		{
			break;
		}
	}

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&contSto);
	cvReleaseImage(&output);
	cvReleaseImage(&conImg);
	cvReleaseHist(&hist);
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");
	cvDestroyWindow("Contour");

	return 1;
}

//PGH法的图片测试
//cmd使用说明:gesrecTest.exe [index = 8] [filename:template] [filename:input] [filename:sample]
int testPGHImg(int argc, char** argv)
{
	IplImage* templateInput;
	IplImage* templateOutput;
	IplImage* input = 0;
	IplImage* output = 0;
	IplImage* sampleImg;//样本图片
	IplImage* conImg;//轮廓图片
	CvScalar s;
	CvSeq* comp;//连通部件
	CvMemStorage* storage;//动态内存
	CvSeq* contour;
	CvMemStorage* contSto;
	CvHistogram* hist = 0;
	int isTemp = 0;
	int matching = 0;
	
	templateInput = cvLoadImage(argv[2], 1);
	input = cvLoadImage(argv[3], 1);

	if(argc == 5)
	{
		if((sampleImg = cvLoadImage(argv[4], 1)) == 0)
		{
			fprintf(stderr, "Could not open sample image\n");
			return 0;
		}
		else
		{
			//获得样本图片的肤色范围
			gesSampleSkinRange(sampleImg, &s);
			cvReleaseImage(&sampleImg);
		}
	}

	cvNamedWindow("Template", 1);
	cvNamedWindow("Input", 1);
	cvNamedWindow("Output", 1);
	cvNamedWindow("TemplateOutput", 1);

	//初始化动态内存
	storage = cvCreateMemStorage(0);
	comp = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);
	contSto = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), contSto);

	conImg = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3);

	//得到模版的PGH直方图
	templateOutput = cvCloneImage(templateInput);
	if(argc == 5)
	{
		gesDetectHandRange(templateInput, templateOutput, comp);
	}
	else
	{
		gesDetectHandRange(templateInput, templateOutput, comp, &s, 1);
	}

	gesFindContours(templateOutput, conImg, &contour, contSto, 1);
	hist = gesCalcContoursPGH(contour);

	output = cvCloneImage(input);
	if(argc == 5)
	{
		gesDetectHandRange(input, output, comp);
	}
	else
	{
		gesDetectHandRange(input, output, comp, &s, 1);
	}

	gesFindContours(output, conImg, &contour, contSto, 1);
	gesMatchContoursPGH(contour, hist);

	cvShowImage("Template", templateInput);
	cvShowImage("TemplateOutput", templateOutput);
	cvShowImage("Input", input);
	cvShowImage("Output", output);
	
	cvWaitKey(0);

	cvReleaseMemStorage(&contSto);
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&input);
	cvReleaseImage(&templateInput);
	cvReleaseImage(&output);
	cvReleaseImage(&templateOutput);
	cvReleaseImage(&conImg);
	cvReleaseHist(&hist);
	cvDestroyWindow("Template");
	cvDestroyWindow("TemplateOutput");
	cvDestroyWindow("Input");
	cvDestroyWindow("Output");

	return 1;
}

int main( int argc, char** argv )
{
	if(argc >= 2)
	{
		if(strcmp(argv[1], "0") == 0)
		{
			if(testSampleHistogram(argc, argv) == 0)
			{
				printf("Could not open file\n");
			}
		}
		else if(strcmp(argv[1], "1") == 0)
		{
			if(testImgDetectHandRange(argc, argv) == 0)
			{
				printf("Could not open file\n");
			}
		}
		else if(strcmp(argv[1], "2") == 0)
		{
			testCamDetectHandRange(argc, argv);
		}
		else if(strcmp(argv[1], "3") == 0)
		{
			getImgFromCAM(argc, argv);
		}
		else if(strcmp(argv[1], "4") == 0)
		{
			testMatchTemplate(argc, argv);
		}
		else if(strcmp(argv[1], "5") == 0)
		{
			testSecaipingheng(argc, argv);
		}
		else if(strcmp(argv[1], "6") == 0)
		{
			testMaxs(argc, argv);
		}
		else if(strcmp(argv[1], "7") == 0)
		{
			testPGH(argc, argv);
		}
		else if(strcmp(argv[1], "8") == 0)
		{
			testPGHImg(argc, argv);
		}
		else
		{
			printf("Command not exists\n");
		}
	}

	return 1;
}