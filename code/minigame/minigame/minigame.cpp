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
const int WIN_WIDTH=600;
const int WIN_HEIGHT=600;
BOOL isFullScreen = FALSE;

// 用户变量定义
CvCapture* capture = 0;
CvMemStorage* compSto;
CvSeq* compSeq;
CvMemStorage* templateContoursSto;
CvSeq* templateContoursSeq;
CvSeq* rectSeq;//存储矩形区域
CvMemStorage* rectSto;
IplImage* camInput;
IplImage* camOutput = 0;
IplImage* camContour;
IplImage* templateContourImg = 0;
CvRect head;//头
CvRect rightHand;//右手
CvRect leftHand;//左手
CvRect lastRightHand;
int headFlag, rightHandFlag, leftHandFlag;
float	yrot;
double translateX, translateY;
int result1, result2;
int rect_num = 0;
int times = 0;

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
	
	loadTemplate("template0.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("template1.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("template2.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("template3.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("template4.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("template5.jpg", templateContourImg, templateContoursSeq);
	//loadTemplate("myskin4.jpg", templateContourImg, templateContoursSeq);/////////////////////与1会混淆
	//loadTemplate("myskin5.jpg", templateContourImg, templateContoursSeq);/////////////////////与3会混淆

	//获得第一帧
	camInput = cvQueryFrame(capture);
	if(!camInput)
	{
		return;
	}
	camContour = cvCreateImage(cvGetSize(camInput), IPL_DEPTH_8U, 3);

	//初始化后面用矩形存储
	rectSto = cvCreateMemStorage(0);
	rectSeq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), rectSto);

	cvNamedWindow("CamInput", 1);
	//cvNamedWindow("CamOutput", 1);
	//cvNamedWindow("CamContour", 1);
}

// OpenCV内存释放
void ReleaseCV(void)
{
	cvReleaseCapture(&capture);
	cvReleaseImage(&camOutput);
	cvReleaseImage(&camContour);
	cvDestroyWindow("CamInout");
	//cvDestroyWindow("CamOutput");
	//cvDestroyWindow("CamContour");
	cvReleaseMemStorage(&templateContoursSto);
	cvReleaseMemStorage(&compSto);
	cvReleaseMemStorage(&rectSto);
}

//开始检测左右手与头
void Detect()
{
	CvConnectedComp* comp;
	CvRect temp;
	int tempFlag;

	headFlag = 0;
	rightHandFlag = 1;
	leftHandFlag = 2;

	//得到当前帧的二值化结果
	gesDetectHandRange(camInput, camOutput, compSeq);

	rect_num = compSeq->total;
		
	if(rect_num < 3)
	{
		//printf("Can't find all three");
		return;
	}

	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 0);
	head = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 1);
	rightHand = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 2);
	leftHand = comp->rect;
	
	//根据矩形中心确定初始时头与左右手位置
	if((head.y+head.height/2) > (rightHand.y+rightHand.height/2))
	{
		tempFlag = headFlag;
		headFlag = rightHandFlag;
		rightHandFlag = tempFlag;

		temp = head;
		head = rightHand;
		rightHand = temp;
	}
	if((head.y+head.height/2) > (leftHand.y+leftHand.height/2))
	{
		tempFlag = headFlag;
		headFlag = leftHandFlag;
		leftHandFlag = tempFlag;

		temp = head;
		head = leftHand;
		leftHand = temp;
	}
	if((rightHand.x+rightHand.height/2) > (leftHand.x+leftHand.height/2))
	{
		tempFlag = rightHandFlag;
		rightHandFlag = leftHandFlag;
		leftHandFlag = tempFlag;

		temp = rightHand;
		rightHand = leftHand;
		leftHand = temp;
	}

	lastRightHand = rightHand;

	cvRectangle(camInput, cvPoint(head.x, head.y),
					cvPoint(head.x + head.width, head.y + head.height), 
					cvScalar(255, 255, 255), 1);
	cvRectangle(camInput, cvPoint(rightHand.x, rightHand.y),
					cvPoint(rightHand.x + rightHand.width, rightHand.y + rightHand.height), 
					cvScalar(0, 255, 0), 1);
	cvRectangle(camInput, cvPoint(leftHand.x, leftHand.y),
					cvPoint(leftHand.x + leftHand.width, leftHand.y + leftHand.height), 
					cvScalar(0, 0, 255), 1);

}

void Track()
{
	CvConnectedComp* comp;
	
	//进行跟踪
	gesTracking(camInput, camOutput, compSeq, rectSeq);

	cvClearSeq(compSeq);
	compSeq = cvCloneSeq(rectSeq);

	rect_num = compSeq->total;

	if(rect_num < 3)
	{
		//printf("Can't find all three");
		return;
	}

	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, headFlag);
	head = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, rightHandFlag);
	rightHand = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, leftHandFlag);
	leftHand = comp->rect;

	//用不同颜色的矩形分别表示各部分
	cvRectangle(camInput, cvPoint(head.x, head.y),
					cvPoint(head.x + head.width, head.y + head.height), 
					cvScalar(255, 255, 255), 1);
	cvRectangle(camInput, cvPoint(rightHand.x, rightHand.y),
					cvPoint(rightHand.x + rightHand.width, rightHand.y + rightHand.height), 
					cvScalar(0, 255, 0), 1);
	cvRectangle(camInput, cvPoint(leftHand.x, leftHand.y),
					cvPoint(leftHand.x + leftHand.width, leftHand.y + leftHand.height), 
					cvScalar(0, 0, 255), 1);

	//计算位移
	translateX -= ((double)(rightHand.x) - lastRightHand.x)/50;
	translateY -= ((double)(rightHand.y) - lastRightHand.y)/50;
	if(translateX > 2)
	{
		translateX = 2;
	}
	else if(translateX < -2)
	{
		translateX = -2;
	}
	if(translateY > 2)
	{
		translateY = 2;
	}
	else if(translateY < -2)
	{
		translateY = -2;
	}
	lastRightHand = rightHand;
}

void Recognize()
{
	IplImage* rightHandOutput;
	IplImage* leftHandOutput;
	IplImage* rightHandContour;
	IplImage* leftHandContour;
	CvPoint2D32f center;

	result1 = -1;
	result2 = -1;

	if(rightHand.width > 0 && rightHand.height > 0)
	{
		rightHandOutput = cvCreateImage(cvSize(rightHand.width, rightHand.height), camOutput->depth, 3);
		center = cvPoint2D32f(rightHand.x+rightHand.width/2, rightHand.y+rightHand.height/2);
		cvGetRectSubPix(camOutput, rightHandOutput, center);

		rightHandContour = cvCreateImage(cvGetSize(rightHandOutput), IPL_DEPTH_8U, 3);
		result1 = gesMatchContoursTemplate2(rightHandOutput, rightHandOutput, templateContoursSeq);	

		cvReleaseImage(&rightHandOutput);
		cvReleaseImage(&rightHandContour);
	}
	
	if(leftHand.width > 0 && leftHand.height > 0)
	{
		leftHandOutput = cvCreateImage(cvSize(leftHand.width, leftHand.height), camOutput->depth, 3);
		center = cvPoint2D32f(leftHand.x+leftHand.width/2, leftHand.y+leftHand.height/2);
		cvGetRectSubPix(camOutput, leftHandOutput, center);

		leftHandContour = cvCreateImage(cvGetSize(leftHandOutput), IPL_DEPTH_8U, 3);
		result2 = gesMatchContoursTemplate2(leftHandOutput, leftHandOutput, templateContoursSeq);

		cvReleaseImage(&leftHandOutput);
		cvReleaseImage(&leftHandContour);
	}

	printf("result1:%d , result2:%d\n", result1, result2);

	if(result2 == 0)
	{
		yrot = ((int)yrot + 10) % 360;
	}
	else if(result2 == 1)
	{
		yrot = ((int)yrot + 350) % 360;
	}
	else if(result2 == 2)
	{

	}
}

void Draw(void)
{
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.5f,0.0f,0.0f); glVertex3f( 0.0f, 0.5f, 0.0f);
	glColor3f(0.0f,0.5f,0.0f); glVertex3f(-0.5f,-0.5f, 0.5f);
	glColor3f(0.0f,0.0f,0.5f); glVertex3f( 0.5f,-0.5f, 0.5f);
	glColor3f(0.0f,0.5f,0.0f); glVertex3f( 0.5f,-0.5f,-0.5f);
	glColor3f(0.0f,0.0f,0.5f); glVertex3f(-0.5f,-0.5f,-0.5f);
	glColor3f(0.0f,0.5f,0.0f); glVertex3f(-0.5f,-0.5f, 0.5f);
	glEnd();
}

// 场景绘制函数
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// 绘制一个旋转的三角锥体
	glTranslatef(translateX,translateY,-10.0f);
	//glRotatef(yrot,0.0f,1.0f,0.0f);

	switch(result2)
	{
	case 1:
		Draw();
	break;

	case 2:
		glPushMatrix();
		glTranslatef(0,1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0,-1,0);
		Draw();
		glPopMatrix();
	break;

	case 3:
		glPushMatrix();
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0,1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0,-1,0);
		Draw();
		glPopMatrix();
	break;

	case 4:
		glPushMatrix();
		glTranslatef(0.5,1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-0.5,1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-0.5,-1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.5,-1,0);
		Draw();
		glPopMatrix();
	break;

	case 5:
		glPushMatrix();
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.5,1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-0.5,1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-0.5,-1,0);
		Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.5,-1,0);
		Draw();
		glPopMatrix();
	break;
	}

	//yrot += 0.15f;

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
	/*camInput = cvQueryFrame(capture);
	if(!camInput)
	{
		return;
	}
	
	cvReleaseImage(&camOutput);
	camOutput = cvCloneImage(camInput);

	gesDetectHandRange(camInput, camOutput, compSeq);
	
	Detect();

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
	glutTimerFunc(33, Timer, 0);*/
	
	//得到当前帧
	camInput = cvQueryFrame(capture);

	//释放前一帧的二值化结果
	cvReleaseImage(&camOutput);
	camOutput = cvCloneImage(camInput);

	if(rect_num < 3 || times % 60 == 0)
	{
		Detect();
		times = times % 60;
	}
	else
	{
		Track();
	}

	times++;

	if(rect_num == 3)
	{
		Recognize();
	}

	cvShowImage("CamInput", camInput);
	glutPostRedisplay();
	//cvShowImage("CamOutput", camOutput);

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