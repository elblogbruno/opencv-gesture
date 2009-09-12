// minigame.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>				// windows的头文件
#include <gl/glut.h>				// Glut的头文件
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

// 定义窗口的标题、宽度、高度
#define WIN_TITLE "Minigame"
const int WIN_WIDTH=400;
const int WIN_HEIGHT=400;
BOOL isFullScreen = FALSE;

// 用户变量定义
CvCapture* capture = 0;
CvMemStorage* compSto;
CvSeq* compSeq;
CvMemStorage* templateContoursSto;
CvSeq* templateContoursSeq;
IplImage* camInput;
IplImage* camOutput = 0;
IplImage* camContour;
IplImage* templateContourImg = 0;
float	yrot;

// OpenGL初始化
void InitGL(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void loadTemplate(char* filename, IplImage* templateContourImg, CvSeq* templateContours)
{
	IplImage* templateInput;
	CvMemStorage* a_templateContourSto;
	CvSeq* a_templateContourSeq;

	a_templateContourSto = cvCreateMemStorage(0);
	a_templateContourSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), a_templateContourSto);

	templateInput = cvLoadImage(filename, 1);
	templateContourImg = cvCreateImage(cvGetSize(templateInput), IPL_DEPTH_8U, 3);

	gesFindContours(templateInput, templateContourImg, &a_templateContourSeq, a_templateContourSto, 1);

	//将一个模版加入
	cvSeqPush(templateContours, a_templateContourSeq);
	printf("ContourNum:%d\n", templateContours->total);

	//释放模版内存
	cvReleaseImage(&templateInput);
	cvReleaseImage(&templateContourImg);
	//cvReleaseMemStorage(&a_templateContourSto);
}

// OpenCV初始化
void InitCV(void)
{
	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return;
	}

	//载入模版
	compSto = cvCreateMemStorage(0);
	compSeq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), compSto);
	templateContoursSto = cvCreateMemStorage(0);
	templateContoursSeq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), templateContoursSto);
	
	loadTemplate("myskin1.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("myskin2.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("myskin3.jpg", templateContourImg, templateContoursSeq);
	//loadTemplate("myskin4.jpg", templateContourImg, templateContoursSeq);/////////////////////与1会混淆
	//loadTemplate("myskin5.jpg", templateContourImg, templateContoursSeq);/////////////////////与3会混淆

	//获得第一帧
	camInput = cvQueryFrame(capture);
	if(!camInput)
	{
		return;
	}
	camContour = cvCreateImage(cvGetSize(camInput), IPL_DEPTH_8U, 3);

	cvNamedWindow("CamOutput", 1);
	cvNamedWindow("CamContour", 1);
}

// OpenCV内存释放
void ReleaseCV(void)
{
	cvReleaseCapture(&capture);
	cvReleaseImage(&camOutput);
	cvReleaseImage(&camContour);
	cvDestroyWindow("CamOutput");
	cvDestroyWindow("CamContour");
	cvReleaseMemStorage(&templateContoursSto);
	cvReleaseMemStorage(&compSto);
}

// 场景绘制函数
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// 绘制一个旋转的三角锥体
	glTranslatef(0.0f,0.0f,-10.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);

	glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);

		glColor3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
		glColor3f(0.0f,0.0f,1.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
		glColor3f(0.0f,1.0f,0.0f); glVertex3f( 1.0f,-1.0f,-1.0f);
		glColor3f(0.0f,0.0f,1.0f); glVertex3f(-1.0f,-1.0f,-1.0f);
		glColor3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
	glEnd();

	yrot+=0.15f;

	glutSwapBuffers ( );	// 交换双缓存
}

// 设置窗口改变大小时的处理函数
void Reshape(int width, int height)
{
	if (height==0)
	{
		height=1;
	}

	// 设置视口
	glViewport(0,0,width,height);

	// 设置投影矩阵
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// 键盘按键动作的处理函数（有对应ASCII码的按键）
void Keyboard (unsigned char key, int x, int y)
{
	switch(key) {
		case 27:				// 按ESC键时退出程序
			ReleaseCV();
			exit (0);
			break;
		default:
			break;
	}
}

// 键盘按键动作的处理函数（无对应ASCII码的按键）
void arrow_keys(int a_keys, int x, int y)
{
	switch(a_keys) {
		case GLUT_KEY_F1:		// 按F1键时切换窗口/全屏模式
			if(isFullScreen)
			{
				glutReshapeWindow(WIN_WIDTH, WIN_HEIGHT);
				glutPositionWindow(30, 30);
				isFullScreen = FALSE;
			}
			else
			{
				glutFullScreen();
				isFullScreen = TRUE;
			}
			break;
		default:
			break;
	}
}

void Timer(int)
{
	camInput = cvQueryFrame(capture);
	if(!camInput)
	{
		return;
	}

	cvReleaseImage(&camOutput);
	camOutput = cvCloneImage(camInput);

	gesDetectHandRange(camInput, camOutput, compSeq);

	int result = gesMatchContoursTemplate2(camOutput, camContour, templateContoursSeq);

	if(result == 0)
	{
		yrot += 1;
	}
	else if(result == 1)
	{
		yrot -= 1;
	}

	cvShowImage("CamOutput", camOutput);
	cvShowImage("CamContour", camContour);

	glutPostRedisplay();
	glutTimerFunc(33, Timer, 0);
}

// 程序主函数
void main(int argc, char** argv)
{
	glutInit(&argc, argv);										// 初始化GLUT库

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	// 初始化显示模式

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);					// 初始化窗口大小
	glutInitWindowPosition(30,30);								// 初始化窗口位置
	glutCreateWindow(WIN_TITLE);								// 建立窗口

	InitGL();					// 初始化OpenGL

	InitCV();					// 初始化OpenCV

	glutDisplayFunc(Display);	// 设置窗口显示时调用的处理函数

	glutReshapeFunc(Reshape);	// 设置窗口改变大小时的处理函数

	glutKeyboardFunc(Keyboard);	// 设置键盘按键动作的处理函数（有对应ASCII码的按键）
	glutSpecialFunc(arrow_keys);// 设置键盘按键动作的处理函数（无对应ASCII码的按键）

	glutTimerFunc(33, Timer, 0);

	glutMainLoop();				// 进入GLUT事件处理循环
}