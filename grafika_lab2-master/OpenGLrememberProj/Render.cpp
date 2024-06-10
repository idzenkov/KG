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

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
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
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
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
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
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

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
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

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

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


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.9, 0.8, 0.8, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.8, 1. };
	GLfloat sh = 0.5f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  


	//������ ��������� ���������� ��������
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
	//����� ��������� ���������� ��������
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






   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
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