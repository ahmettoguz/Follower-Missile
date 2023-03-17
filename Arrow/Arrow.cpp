/*********
   CTIS164 - Template Source Program
----------
STUDENT : Ahmet Oðuz Ergin
SECTION : 3
HOMEWORK: 3
----------
PROBLEMS:
----------
ADDITIONAL FEATURES:
-> Target object rotates around itself according to its movement (ex. if goes down in linear mode, rotate clockwise.
	 if goes up, rotate counterclockwise).

-> If target object reach the windows frame its speed is changing randomly in linear mode (new dx dy values randomly)

-> There are 3 path modes: Linear, Orbital and Harmonic

-> When right click is pressed, new movement properties, path mode and coordinates (clicked location) is assigning
	 to target object (if path mode is orbital, and if the clicked location is out of the windows frame,
	 it automatically insert target into frames ).

-> As in the sample video, paths of the target object is displayed (in orbital mode radius is changing randomly).

->	My arrow (flying object) has fires which get visible after some time and that fires	are
	  getting bigger and changing its color with time.

-> if distance is close enough target object color is changing

-> When 'n' is clicked path mode is changing.

-> It is possible to change arrow speed with 'f' and 's' keys.(faster,slower).

-> You can change chase mode to auto/manuel with 'm' key.
	In manuel mode you can rotate and go forward with arrow keys

*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include"vec.h"

#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 750

#define TIMER_PERIOD  16 // Period for the timer.
#define TIMER_ON      1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

typedef struct {
	float r, g, b;
	double t;
}color_t;

typedef struct {
	char linearorbitalharmonic; // 1=linear,2orbital,3harmonic
	bool leftright; // true means goes to left, false means goes to right
	bool updown; // true means goes to up, false means goes to down
	double dx, dy;
	double angle;
	int radius;
	int distance2origin;
}target_move;

typedef struct {
	vec_t p; //position
	polar_t polar;
	int r; //radius
	target_move move;
	color_t c; //color
}target_t;

typedef struct {
	vec_t p;	//position
	double angle;
	color_t fire;
	double speed;
	bool active;
}arrow_t;

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height
double fireleftmagnit, firemidmagnit, firemagnit;
int count = 0;
bool mode_auto_manuel;  //if true means auto
vec_t endv;
target_t t; //target
arrow_t arrow;

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char* string, void* font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void vertex(double x, double y, vec_t position, double angle) {
	double xp = (x * cos(angle * D2R) - y * sin(angle * D2R)) + position.x;
	double yp = (x * sin(angle * D2R) + y * cos(angle * D2R)) + position.y;
	glVertex2f(xp, yp);
}

void dispbackrec()
{
	glBegin(GL_QUADS);
	glColor3ub(213, 213, 250);
	glVertex2f(-750, -375);
	glVertex2f(750, -375);

	glColor3ub(130, 130, 230);
	glVertex2f(750, 375);
	glVertex2f(-750, 375);

	glEnd();
}

void display_Paths(target_t t)
{
	//orbital
	if (t.move.linearorbitalharmonic == 2)
	{
		//red last orbit
		{
			glLineWidth(3);
			glColor3f(1, 0, 0);
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i <= 360; i++)
				glVertex2f(t.move.radius * cos(i * D2R), t.move.radius * sin(i * D2R));
			glEnd();
		}

		//purple current orbit
		{
			glColor3ub(230, 6, 197);
			glEnable(GL_LINE_STIPPLE);
			glLineWidth(7);
			glLineStipple(1, 0x00F0);
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i <= 360; i++)
				glVertex2f(t.move.distance2origin * cos(i * D2R), t.move.distance2origin * sin(i * D2R));
			glEnd();
			glDisable(GL_LINE_STIPPLE);
		}
	}

	//linear
	else if (t.move.linearorbitalharmonic == 1)
	{
		glLineWidth(2);
		glColor3ub(230, 6, 197);
		glBegin(GL_LINES);
		glVertex2f(t.p.x, t.p.y);
		if (t.move.leftright == true && t.move.updown == true)
			glVertex2f(t.p.x - t.move.dx * 1000, t.p.y + t.move.dy * 1000);
		else if (t.move.leftright == true && t.move.updown == false)
			glVertex2f(t.p.x - t.move.dx * 1000, t.p.y - t.move.dy * 1000);
		else if (t.move.leftright == false && t.move.updown == true)
			glVertex2f(t.p.x + t.move.dx * 1000, t.p.y + t.move.dy * 1000);
		else if (t.move.leftright == false && t.move.updown == false)
			glVertex2f(t.p.x + t.move.dx * 1000, t.p.y - t.move.dy * 1000);
		glEnd();
	}

	//harmonic
	else if (t.move.linearorbitalharmonic == 3)
	{
		float x, y;
		glColor3ub(230, 6, 197);
		glBegin(GL_LINE_STRIP);
		for (x = -700; x <= 700; x++)
		{
			y = 210 * sin(1 * x * D2R);
			glVertex2f(x, y);
		}
		glEnd();
	}
}

void move_Target(target_t* t)
{
	//rotate target around its own axis
	{
		if (t->move.updown == true)
			t->polar.angle += 3;
		if (t->move.updown == false)
			t->polar.angle -= 3;
	}

	//orbital movement
	if (t->move.linearorbitalharmonic == 2)
	{
		//reduce or increase the radius according to coordinates
		{
			t->move.distance2origin = sqrt((pow(0 - t->p.x, 2)) + (pow(0 - t->p.y, 2)));

			if (t->move.distance2origin > 350)
				t->move.distance2origin = 350;

			if (t->move.distance2origin <= t->move.radius && !(t->move.distance2origin >= t->move.radius - 1.2))
				t->move.distance2origin += 1;
			if (t->move.distance2origin >= t->move.radius && !(t->move.distance2origin <= t->move.radius + 1.2))
				t->move.distance2origin -= 1;

			if (t->move.distance2origin >= t->move.radius - 1.5 && t->move.distance2origin <= t->move.radius + 1.5)
				t->move.distance2origin = t->move.radius;
		}

		//change angle
		{
			if (t->move.leftright == true)
				t->move.angle += 1.3;
			if (t->move.leftright == false)
				t->move.angle -= 1.3;

			if (t->move.angle > 360)
				t->move.angle -= 360;
			if (t->move.angle < 0)
				t->move.angle += 360;
		}

		t->p.x = t->move.distance2origin * (cos(t->move.angle * D2R));
		t->p.y = t->move.distance2origin * (sin(t->move.angle * D2R));
	}

	//linear movement
	else if (t->move.linearorbitalharmonic == 1)
	{
		//return the target if reached the frames
		{
			if (t->p.y >= 325)
			{
				t->move.updown = false;
				t->move.dx = rand() % ((5 + 1) - 3) + 3;
				t->move.dy = rand() % ((5 + 1) - 3) + 3;
			}
			if (t->p.y <= -325)
			{
				t->move.updown = true;
				t->move.dx = rand() % ((5 + 1) - 3) + 3;
				t->move.dy = rand() % ((5 + 1) - 3) + 3;
			}
			if (t->p.x >= 650)
			{
				t->move.leftright = true;
				t->move.dx = rand() % ((5 + 1) - 3) + 3;
				t->move.dy = rand() % ((5 + 1) - 3) + 3;
			}
			if (t->p.x <= -650)
			{
				t->move.leftright = false;
				t->move.dx = rand() % ((5 + 1) - 3) + 3;
				t->move.dy = rand() % ((5 + 1) - 3) + 3;
			}
		}

		//add dx and dy
		{
			if (t->move.leftright == true)
				t->p.x -= t->move.dx;
			if (t->move.leftright == false)
				t->p.x += t->move.dx;
			if (t->move.updown == true)
				t->p.y += t->move.dy;
			if (t->move.updown == false)
				t->p.y -= t->move.dy;
		}
	}

	//harmonic movement
	else if (t->move.linearorbitalharmonic == 3)
	{
		if (t->move.leftright == true)
			t->p.x--;
		else
			t->p.x++;
		t->p.y = 210 * sin(1 * t->p.x * D2R);

		//check frames
		{
			if (t->p.x >= 650)
				t->move.leftright = true;
			if (t->p.x <= -650)
				t->move.leftright = false;
		}
	}
}

void disp_Target(target_t t)
{
	//body circle
	{
		glLineWidth(3);
		glColor3ub(t.c.r, t.c.g, t.c.b);
		circle(t.p.x, t.p.y, 40);
		glColor3f(0, 0, 0);
		circle_wire(t.p.x, t.p.y, 40 + 2);
	}

	//triangles
	{
		//up triangle
		{
			glColor3ub(0, 145, 0);
			glBegin(GL_TRIANGLES);
			vertex(0, 70, t.p, t.polar.angle);
			vertex(-20, 0, t.p, t.polar.angle);
			vertex(20, 0, t.p, t.polar.angle);
			glEnd();
		}

		//down triangle
		{
			glColor3ub(0, 145, 0);
			glBegin(GL_TRIANGLES);
			vertex(0, -70, t.p, t.polar.angle);
			vertex(-20, 0, t.p, t.polar.angle);
			vertex(20, 0, t.p, t.polar.angle);
			glEnd();
		}

		//left triangle
		{
			glColor3ub(0, 145, 0);
			glBegin(GL_TRIANGLES);
			vertex(-70, 0, t.p, t.polar.angle);
			vertex(0, 20, t.p, t.polar.angle);
			vertex(0, -20, t.p, t.polar.angle);
			glEnd();
		}

		//right triangle
		{
			glColor3ub(0, 145, 0);
			glBegin(GL_TRIANGLES);
			vertex(70, 0, t.p, t.polar.angle);
			vertex(0, 20, t.p, t.polar.angle);
			vertex(0, -20, t.p, t.polar.angle);
			glEnd();
		}
	}

	//circle into 
	{
		glColor3ub(10, 5, 80);
		circle(t.p.x, t.p.y, 12);
	}
}

void disp_Arrow(arrow_t* a, vec_t* endv)
{
	if (a->active == true)
	{
		*endv = addV(a->p, pol2rec({ 100, a->angle }));
		vec_t upbody = addV(a->p, pol2rec({ 70, a->angle }));
		vec_t leftupv = addV(a->p, pol2rec({ 62, a->angle + 35 }));
		vec_t rightupv = addV(a->p, pol2rec({ 62, a->angle - 35 }));

		vec_t midbody = addV(a->p, pol2rec({ 52, a->angle }));
		vec_t downbody = addV(a->p, pol2rec({ 20, a->angle }));
		vec_t leftupp = addV(a->p, pol2rec({ 42, a->angle + 40 }));
		vec_t leftdownn = addV(a->p, pol2rec({ 32, a->angle + 90 }));
		vec_t rightupp = addV(a->p, pol2rec({ 42, a->angle - 40 }));
		vec_t rightdownn = addV(a->p, pol2rec({ 32, a->angle - 90 }));

		vec_t fireleftconst = addV(a->p, pol2rec({ 32, a->angle + 130 }));
		vec_t firerightconst = addV(a->p, pol2rec({ 32, a->angle - 130 }));
		vec_t fireleft = addV(a->p, pol2rec({ fireleftmagnit, a->angle + 160 }));
		vec_t fireright = addV(a->p, pol2rec({ fireleftmagnit, a->angle - 160 }));
		vec_t firemid = addV(a->p, pol2rec({ firemidmagnit, a->angle + 180 }));

		vec_t firemidbottom = addV(a->p, pol2rec({ firemagnit, a->angle + 180 }));
		vec_t firebottomleft = addV(a->p, pol2rec({ 10, a->angle + 145 }));
		vec_t firebottomright = addV(a->p, pol2rec({ 10, a->angle - 145 }));

		//main line (body)
		{
			glLineWidth(10);
			glColor3ub(255, 0, 0);
			glBegin(GL_LINES);
			glVertex2f(a->p.x, a->p.y);
			glVertex2f(upbody.x, upbody.y);
			glEnd();
		}

		//up part of arrow
		{
			glColor3f(0, 0.3, 1);
			glBegin(GL_POLYGON);
			glVertex2f(endv->x, endv->y);
			glVertex2f(leftupv.x, leftupv.y);
			glVertex2f(upbody.x, upbody.y);
			glVertex2f(rightupv.x, rightupv.y);
			glEnd();

			//frames
			{
				glLineWidth(5);
				glColor3f(0, 0, 0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(endv->x, endv->y);
				glVertex2f(leftupv.x, leftupv.y);
				glVertex2f(upbody.x, upbody.y);
				glVertex2f(rightupv.x, rightupv.y);
				glEnd();
			}
		}

		//down part of arrow
		{
			glLineWidth(10);
			glColor3f(0, 0.3, 1);
			glBegin(GL_POLYGON);
			glVertex2f(midbody.x, midbody.y);
			glVertex2f(leftupp.x, leftupp.y);
			glVertex2f(leftdownn.x, leftdownn.y);
			glVertex2f(downbody.x, downbody.y);
			glVertex2f(rightdownn.x, rightdownn.y);
			glVertex2f(rightupp.x, rightupp.y);
			glEnd();

			//frames
			{
				glLineWidth(5);
				glColor3f(0, 0, 0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(midbody.x, midbody.y);
				glVertex2f(leftupp.x, leftupp.y);
				glVertex2f(leftdownn.x, leftdownn.y);
				glVertex2f(downbody.x, downbody.y);
				glVertex2f(rightdownn.x, rightdownn.y);
				glVertex2f(rightupp.x, rightupp.y);
				glEnd();
			}
		}

		//fire which is get bigger
		{
			glLineWidth(10);
			glColor4ub(a->fire.r, a->fire.g, a->fire.b, a->fire.t);
			glBegin(GL_POLYGON);
			glVertex2f(a->p.x, a->p.y);
			glVertex2f(fireleftconst.x, fireleftconst.y);
			glVertex2f(fireleft.x, fireleft.y);
			glVertex2f(firemid.x, firemid.y);
			glVertex2f(fireright.x, fireright.y);
			glVertex2f(firerightconst.x, firerightconst.y);
			glEnd();

			glBegin(GL_POLYGON);
			glVertex2f(firebottomleft.x, firebottomleft.y);
			glVertex2f(firemidbottom.x, firemidbottom.y);
			glVertex2f(firebottomright.x, firebottomright.y);
			glEnd();
		}
	}
}

void incre_magnit(double* l, double* m, arrow_t* a)
{
	if (*l <= 80)
	{
		(*l) += 0.1;
		(*m) += 0.1;
	}

	if (a->fire.t < 254)
		a->fire.t += 2;
	if (a->fire.g > 1)
		a->fire.g -= 2;
	else if (a->fire.r > 1)
		a->fire.r -= 2;
}

void chase(arrow_t* a, target_t t)
{
	if (mode_auto_manuel == true)
	{
		vec_t velvector = mulV(a->speed, unitV(subV(a->p, t.p)));

		//find angle
		{
			double dy = t.p.y - a->p.y;
			double dx = t.p.x - a->p.x;
			a->angle = atan2(dy, dx) * V_R2D;
		}
		a->p = subV(a->p, velvector);
	}
}

void move_arrow_manuel(arrow_t* a)
{
	if (mode_auto_manuel == false)
	{
		double dx = a->speed * cos(a->angle * D2R);
		double dy = a->speed * sin(a->angle * D2R);

		a->p.x += dx;
		a->p.y += dy;

		//check if arrow exceed the frames
		{
			if (a->p.x > 600 || a->p.x < -600)
				a->p.x -= dx;

			if (a->p.y > 275 || a->p.y < -275)
				a->p.y -= dy;
		}
	}
}

void check_hit(arrow_t* a, target_t* t, vec_t e, int* count)
{
	if (a->active == true)
	{
		vec_t distv;
		distv = subV(e, t->p);
		double dist = sqrt(pow(distv.x, 2) + pow(distv.y, 2));

		//if distance is less than 100 it means top of arrow hits the target, so need to be disappear
		if (dist < 60)
		{
			t->c.r = 255;
			t->c.g = 0;
			t->c.b = 0;
		}
		else
		{
			t->c.r = 255;
			t->c.g = 145;
			t->c.b = 60;
		}

		if (dist < 10)
		{
			a->active = false;
			(*count)++;
		}
	}
	else
	{
		t->c.r = 255;
		t->c.g = 145;
		t->c.b = 60;
	}
}

void disp_info(target_t t)
{
	//box
	{
		glColor4f(255, 0, 0, 0.3);
		glRectf(-700, -220, -350, -375);

		glLineWidth(7);
		glColor4f(1, 0, 0, 0.8);
		glBegin(GL_LINE_LOOP);
		glVertex2f(-698, -220);
		glVertex2f(-347, -220);
		glVertex2f(-347, -372);
		glVertex2f(-698, -372);
		glEnd();
	}

	//messages
	{
		glLineWidth(1.5);
		glColor3f(0, 0.1, 1);
		if (mode_auto_manuel == false)
			vprint(-630, -250, GLUT_BITMAP_HELVETICA_18, "Press M To chase Auto");
		else
			vprint(-630, -250, GLUT_BITMAP_HELVETICA_18, "Press M To chase Manuel");

		if (t.move.linearorbitalharmonic == 1)
			vprint(-613, -272, GLUT_BITMAP_HELVETICA_18, "Press N to Orbital path");
		else if (t.move.linearorbitalharmonic == 2)
			vprint(-613, -272, GLUT_BITMAP_HELVETICA_18, "Press N to Harmonic path");
		else if (t.move.linearorbitalharmonic == 3)
			vprint(-613, -272, GLUT_BITMAP_HELVETICA_18, "Press N to Linear path");

		vprint(-630, -295, GLUT_BITMAP_HELVETICA_18, "Press S to move Slower");
		vprint(-625, -315, GLUT_BITMAP_HELVETICA_18, "Press F to move Faster");

		vprint(-630, -340, GLUT_BITMAP_HELVETICA_18, "Left Click to Spawn Arrow");
		vprint(-680, -360, GLUT_BITMAP_HELVETICA_18, "Right Click to Spawn Target randomly");

	}
}

//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	//display background big rectangle
	dispbackrec();

	//display target
	disp_Target(t);

	//display paths of target motion
	display_Paths(t); //make key to open and close that

	//display flying object arrow
	disp_Arrow(&arrow, &endv);

	//display info box
	disp_info(t);

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	//change speed
	{
		if ((key == 'f' || key == 'F') && arrow.speed <= 6)
			arrow.speed += 1;
		if ((key == 's' || key == 'S') && arrow.speed > 2)
			arrow.speed -= 1;
	}

	//change mode
	{
		if (key == 'M' || key == 'm')
			mode_auto_manuel = !mode_auto_manuel;
	}

	//change path mode
	if (key == 'n' || key == 'N')
	{
		if (t.move.linearorbitalharmonic == 1)
			t.move.linearorbitalharmonic = 2;
		else if (t.move.linearorbitalharmonic == 2)
		{
			t.move.linearorbitalharmonic = 3;
			t.p.y = 210 * sin(1 * t.p.x * D2R);
		}
		else if (t.move.linearorbitalharmonic == 3)
			t.move.linearorbitalharmonic = 1;
		t.move.leftright = rand() % ((1 + 1) - 0) + 0;
		t.move.updown = rand() % ((1 + 1) - 0) + 0;
		t.move.dx = rand() % ((7 + 1) - 4) + 4;
		t.move.dy = rand() % ((7 + 1) - 4) + 4;
		t.move.radius = rand() % ((250 + 1) - 180) + 180;
		t.move.angle = atan2(t.p.y, t.p.x) * V_R2D;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = true; break;
	case GLUT_KEY_DOWN: down = true; break;
	case GLUT_KEY_LEFT: left = true; break;
	case GLUT_KEY_RIGHT: right = true; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	// GLUT to OpenGL coordinate conversion:
	x = x - winWidth / 2;
	y = winHeight / 2 - y;

	//change location of the target
	if (stat == GLUT_DOWN && button == GLUT_RIGHT_BUTTON)
	{
		//initialize target as clicked location
		{
			t.p.x = x;
			t.p.y = y;
			t.polar.angle = 0;
			t.move.linearorbitalharmonic = rand() % ((3 + 1) - 1) + 1;
			t.move.leftright = rand() % ((1 + 1) - 0) + 0;
			t.move.updown = rand() % ((1 + 1) - 0) + 0;
			t.move.dx = rand() % ((7 + 1) - 4) + 4;
			t.move.dy = rand() % ((7 + 1) - 4) + 4;
			t.move.radius = rand() % ((250 + 1) - 180) + 180;
			t.move.angle = atan2(t.p.y, t.p.x) * V_R2D;
		}
	}

	//change locatiion of the arrow
	if (stat == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		arrow.active = true;
		arrow.p.x = x;
		arrow.p.y = y;
		arrow.fire.r = 255;
		arrow.fire.g = 255;
		arrow.fire.t = 30;
		fireleftmagnit = 32;
		firemagnit = 40;
		firemidmagnit = 5;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.


	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.


	// to refresh the window it calls display() function
	glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.

	//move target
	move_Target(&t);

	//chase arrow to target auto motion
	chase(&arrow, t);

	//increment the magnititude of the fires
	incre_magnit(&fireleftmagnit, &firemagnit, &arrow);

	//calculate distance between 2 vector and decide hit
	check_hit(&arrow, &t, endv, &count);

	//move arrow manuel
	{
		if (mode_auto_manuel == false)
		{
			if (left)
				arrow.angle += 1.2;
			if (right)
				arrow.angle -= 1.2;
		}

		if (up)
			move_arrow_manuel(&arrow);
	}

	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()
}
#endif

void Init() {

	//initialize target
	{
		t.p.x = 0;
		t.p.y = 0;
		t.r = 40;
		t.polar.angle = 0;
		t.polar.magnitude = 40;
		t.c.r = 255;
		t.c.g = 145;
		t.c.b = 60;
	}

	//initialize target movement
	{
		t.move.linearorbitalharmonic = 1;
		t.move.leftright = true;
		t.move.updown = true;
		t.move.dx = 5;
		t.move.dy = 3;
		t.move.angle = 0;
		t.move.radius = 100;
		t.move.distance2origin = sqrt((pow(0 - t.p.x, 2)) + (pow(0 - t.p.y, 2)));
	}

	//initialize arrow
	{
		arrow.speed = 2.1;
		arrow.p.x = 0;
		arrow.p.y = 0;
		arrow.angle = 0;
		count = 0;

		mode_auto_manuel = true;
	}

	//time function for random
	srand(time(NULL));

	// Smoothing shapes
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(45, 50);
	glutCreateWindow("Smart Arrow By Ahmet Oðuz Ergin");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	//glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	//glutMotionFunc(onMoveDown);
	//glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}