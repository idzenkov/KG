#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

struct Point {
	double x;
	double y;
	double z;
};
void makefigure(GLUtesselator * tess, double arr[8][3], double arr1[8][3]);

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.0, 0.0, 0.0, 1 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 1 };
		GLfloat spec[] = { 0.7, 0.7, 0.7, 1 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





void Render(OpenGL *ogl)
{

	

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.9, 0.8, 0.8, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.8, 1. };
	GLfloat sh = 0.5f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	/*double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();*/
	//конец рисования квадратика станкина
	void CALLBACK tessBeginCB(GLenum which);
	void CALLBACK tessVertexCB(const GLvoid * data);
	void quads(double arr[8][3], double arr1[8][3]);
	void figure(GLUtesselator * tess, double arr[8][3]);
	void drawNormal(Point v[8]);

	double x = 0;
	double y = 0;
	double z = 0;

	double arr[8][3] = { {x,y,z},{x += 5,y += 2,z} ,{x -= 1,y += 6,z},{x += 3,y -= 5,z},{x += 7,y += 1,z},{x,y -= 7,z},{x -= 8,y += 3,z},{x -= 4,y -= 7,z} };

	x = 0;
	y = 0;
	z = 0;
	Point narr[9] = { { x,y,z },{ x += 5,y += 2,z } ,{ x -= 1,y += 6,z },{ x += 3,y -= 5,z },{ x += 7,y += 1,z },{ x,y -= 7,z},{ x -= 8,y += 3,z},{ x -= 4,y -= 7,z},{0,0,0} };

	GLUtesselator* tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall*)())tessBeginCB);
	gluTessCallback(tess, GLU_TESS_END, glEnd);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall*)())glVertex3dv);
	

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	x = 0;
	y = 0;
	z += 1;

	double arr1[8][3] = { {x,y,z},{x += 5,y += 2,z} ,{x -= 1,y += 6,z},{x += 3,y -= 5,z},{x += 7,y += 1,z},{x,y -= 7,z},{x -= 8,y += 3,z},{x -= 4,y -= 7,z} };
	Point narr1[8] = { { x,y,z },{ x += 5,y += 2,z } ,{ x -= 1,y += 6,z },{ x += 3,y -= 5,z },{ x += 7,y += 1,z },{ x,y -= 7,z},{ x -= 8,y += 3,z},{ x -= 4,y -= 7,z} };

	makefigure(tess, arr, arr1);
	gluDeleteTess(tess);




	//drawNormal(narr);






   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

void CALLBACK tessBeginCB(GLenum which) {

	glBegin(which);
}

void CALLBACK tessVertexCB(const GLvoid* data)
{
	// cast back to double type
	const GLdouble* ptr = (const GLdouble*)data;

	glVertex3dv(ptr);

}

void figure(GLUtesselator* tess, double arr[8][3]) {
	gluTessNormal(tess, 0.0, 0.0, 1.0);
	gluBeginPolygon(tess);
	gluTessBeginContour(tess);
	glColor3d(0.1, 0.1, 0.1);
	gluTessVertex(tess, arr[0], arr[0]);
	gluTessVertex(tess, arr[1], arr[1]);
	gluTessVertex(tess, arr[2], arr[2]);
	gluTessVertex(tess, arr[3], arr[3]);
	gluTessVertex(tess, arr[4], arr[4]);
	gluTessVertex(tess, arr[5], arr[5]);
	gluTessVertex(tess, arr[6], arr[6]);
	gluTessVertex(tess, arr[7], arr[7]);
	gluTessEndContour(tess);
	gluEndPolygon(tess);
	gluTessNormal(tess, arr[0][0], arr[0][0], arr[0][2]);
	gluTessNormal(tess, arr[1][0], arr[1][0], arr[1][2]);
	gluTessNormal(tess, arr[2][0], arr[2][0], arr[2][2]);
	gluTessNormal(tess, arr[3][0], arr[3][0], arr[3][2]);
	gluTessNormal(tess, arr[4][0], arr[4][0], arr[4][2]);
	gluTessNormal(tess, arr[5][0], arr[5][0], arr[5][2]);
	gluTessNormal(tess, arr[6][0], arr[6][0], arr[6][2]);
	gluTessNormal(tess, arr[7][0], arr[7][0], arr[7][2]);
	gluTessNormal(tess, arr[0][0], arr[0][0], arr[0][2]);
	
	
}

void quads(GLUtesselator* tess,double arr[8][3], double arr1[8][3])
{
	gluTessNormal(tess, 1.0, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, texId);
	for (int i = 0; i < 8; i++)
	{
		/*glBegin(GL_QUADS);*/
		gluBeginPolygon(tess);
		gluTessBeginContour(tess);
		glColor3d(0.1, 0.1, 0.1);
		if (i == 4) {
			glColor3d(0, 1, 0);
		}
		if (i == 7)
		{
			
			glColor3d(0.8, 0, 0.8);
			
			
			gluTessVertex(tess, arr[i], arr[i]);
			gluTessVertex(tess, arr1[i], arr1[i]);
			gluTessVertex(tess, arr1[0], arr1[0]);
			gluTessVertex(tess, arr[0], arr[0]);
			/*glVertex3dv(arr[i]);
			glVertex3dv(arr1[i]);
			glVertex3dv(arr1[0]);
			glVertex3dv(arr[0]);*/
			
			gluTessEndContour(tess);
			gluEndPolygon(tess);
			
		}
		

		gluTessVertex(tess, arr[i], arr[i]);
		gluTessVertex(tess, arr1[i], arr1[i]);
		gluTessVertex(tess, arr1[i+1], arr1[i+1]);
		gluTessVertex(tess, arr[i+1], arr[i+1]);
		/*glVertex3dv(arr[i]);
		glVertex3dv(arr1[i]);
		glVertex3dv(arr1[0]);
		glVertex3dv(arr[0]);*/

		gluTessEndContour(tess);
		gluEndPolygon(tess);
		
		/*glVertex3dv(arr[i]);
		glVertex3dv(arr1[i]);
		glVertex3dv(arr1[i + 1]);
		glVertex3dv(arr[i + 1]);
		glEnd();*/
	}
}

Point calculateNormal(Point a, Point b, bool isLeft) {
	Point r;

	r.x = (a.y * b.z - b.y * a.z);
	r.y = (-a.x * b.z + b.x * a.z);
	r.z = (a.x * b.y - b.x * a.y);

	double n = sqrt(pow(r.x, 2) + pow(r.y, 2) + pow(r.z, 2));
	r.x = -r.x / n;
	r.y = -r.y / n;
	r.z = -r.z / n;


	return r;
}

void drawNormal(Point v[8])
{
	// Begin
	for (int i = 0; i < 8; i++)
	{
		Point n = calculateNormal({ v[i + 1].x - v[i].x, v[i + 1].y - v[i].y, 0 }, { 0, 0, 1 }, true);

		glBegin(GL_LINES);
		glColor3d(0.8, 0.3, 0.3);
		glVertex3d(v[i].x, v[i].y, 0);
		glVertex3d(v[i].x + n.x, v[i].y + n.y, 0);
		glEnd();

		glBegin(GL_LINES);
		glColor3d(0.8, 0.3, 0.3);
		glVertex3d(v[i].x, v[i].y, 1);
		glVertex3d(v[i].x + n.x, v[i].y + n.y, 1);
		glEnd();

	}

	// End
	for (int i = 1; i < 9; i++)
	{
		Point n = calculateNormal({ v[i - 1].x - v[i].x, v[i - 1].y - v[i].y, 0 }, { 0, 0, 1 }, true);

		glBegin(GL_LINES);
		glColor3d(0.8, 0.3, 0.3);
		glVertex3d(v[i].x, v[i].y, 0);
		glVertex3d(v[i].x - n.x, v[i].y - n.y, 0);
		glEnd();

		glBegin(GL_LINES);
		glColor3d(0.8, 0.3, 0.3);
		glVertex3d(v[i].x, v[i].y, 1);
		glVertex3d(v[i].x - n.x, v[i].y - n.y, 1);
		glEnd();

	}
	for (int i = 0; i < 9; i++)
	{
		glBegin(GL_LINES);
		glColor3d(0.8, 0.3, 0.3);
		glVertex3d(v[i].x, v[i].y, 0);
		glVertex3d(v[i].x, v[i].y, -1);
		glEnd();

		glBegin(GL_LINES);
		glColor3d(0.8, 0.3, 0.3);
		glVertex3d(v[i].x, v[i].y, 1);
		glVertex3d(v[i].x, v[i].y, 1 + 1);
		glEnd();

	}
}
void makefigure(GLUtesselator* tess, double arr[8][3], double arr1[8][3]) {
	figure(tess, arr);
	quads(tess,arr, arr1);
	figure(tess, arr1);
}