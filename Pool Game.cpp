// Pool Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdafx.h"
#include<glut.h>
#include<math.h>
#include"simulation.h"
#include <time.h>

const float M_PI = 3.14159265358979323846;  /* pi */

//cue variables
float gCueAngle = 0.0;
float gCuePower = 0.25;
bool gCueControl[4] = {false,false,false,false};
float gCueAngleSpeed = 2.0f; //radians per second
float gCuePowerSpeed = 0.25f;
float gCuePowerMax = 0.75;
float gCuePowerMin = 0.1;
float gCueBallFactor = 8.0;
bool gDoCue = true;
int player = 0;

//camera variables
vec3 gCamPos(0.0,0.7,2.1);
vec3 gCamLookAt(0.0,0.0,0.0);
bool gCamRotate = true;
float gCamRotSpeed = 0.2;
float gCamMoveSpeed = 0.5;
bool gCamL = false;
bool gCamR = false;
bool gCamU = false;
bool gCamD = false;
bool gCamZin = false;
bool gCamZout = false;

//rendering options
#define DRAW_SOLID	(0)

void DoCamera(int ms)
{
	static const vec3 up(0.0,1.0,0.0);

	if(gCamRotate)
	{
		if(gCamL)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localL = up.Cross(camDir);
			vec3 inc = (localL* ((gCamRotSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir + inc;
		}
		if(gCamR)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = up.Cross(camDir);
			vec3 inc = (localR* ((gCamRotSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir - inc;
		}
		if(gCamU)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp* ((gCamMoveSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir + inc;
		}
		if(gCamD)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp* ((gCamMoveSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir - inc;
		}		
	}
	else
	{
		if(gCamL)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localL = up.Cross(camDir);
			vec3 inc = (localL* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamR)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 inc = (localR* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamU)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localUp = localR.Cross(camDir);
			vec3 inc = (localUp* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamD)
		{
			vec3 camDir = (gCamLookAt - gCamPos).Normalised();
			vec3 localR = camDir.Cross(up);
			vec3 localDown = camDir.Cross(localR);
			vec3 inc = (localDown* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
	}

	if(gCamZin)
	{
		vec3 camDir = (gCamLookAt - gCamPos).Normalised();
		vec3 inc = (camDir* ((gCamMoveSpeed*ms)/1000.0) );
		gCamPos += inc;
		gCamLookAt += inc;
	}
	if(gCamZout)
	{
		vec3 camDir = (gCamLookAt - gCamPos).Normalised();
		vec3 inc = (camDir* ((gCamMoveSpeed*ms)/1000.0) );
		gCamPos -= inc;
		gCamLookAt -= inc;
	}
}


void drawBitmapText(char *string, float x, float y, float z)
{
	char *c;
	glRasterPos3f(x, y, z);

	for (c = string; *c != NULL; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
}


void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set camera
	glLoadIdentity();
	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);
	
	if (gMenu.drawMenu)
	{
		if(gMenu.menuSelection == 1) glColor3f(1.0, 0, 0);
		drawBitmapText("Course #1", -1, 0, 0);
		glColor3f(1.0, 1.0, 1.0);

		if (gMenu.menuSelection == 2) glColor3f(1.0, 0, 0);
		drawBitmapText("Course #2", -0.2, 0, 0);
		glColor3f(1.0, 1.0, 1.0);

		if (gMenu.menuSelection == 3) glColor3f(1.0, 0, 0);
		drawBitmapText("Course #3", 0.6, 0, 0);
		glColor3f(1.0, 1.0, 1.0);
	}
	else 
	{
		//draw the ball
		glColor3f(1.0, 1.0, 1.0);
		for (int i = 0; i < NUM_BALLS; i++)
		{
			glPushMatrix();
			glTranslatef(gTable.balls[i].position(0), (BALL_RADIUS / 2.0), gTable.balls[i].position(1));

			int rat = i / 3 + 1;
			if (i % 3 == 0) {
				glColor3f(1.0 / rat, 0.0, 0.0);
			}
			else if (i % 3 == 1)
			{
				glColor3f(0.0, 0.0, 1.0 / rat);
			}
			else
			{
				glColor3f(0.0, 1.0 / rat, 0.0);
			}

#if DRAW_SOLID
			glutSolidSphere(gTable.balls[i].radius, 32, 32);
#else
			glutWireSphere(gTable.balls[i].radius, 12, 12);
#endif
			glPopMatrix();
			glColor3f(0.0, 0.0, 1.0);
		}
		glColor3f(1.0, 1.0, 1.0);

		//draw the table
		for (int i = 0; i < NUM_CUSHIONS; i++)
		{
			glBegin(GL_LINE_LOOP);
			glVertex3f(gTable.cushions[i].vertices[0](0), 0.0, gTable.cushions[i].vertices[0](1));
			glVertex3f(gTable.cushions[i].vertices[0](0), 0.1, gTable.cushions[i].vertices[0](1));
			glVertex3f(gTable.cushions[i].vertices[1](0), 0.1, gTable.cushions[i].vertices[1](1));
			glVertex3f(gTable.cushions[i].vertices[1](0), 0.0, gTable.cushions[i].vertices[1](1));
			glEnd();
		}

		//Draw hole
		for (int j = 0; j < NUM_HOLES; j++)
		{
			if(!gTable.holes[j].isTarget)
				glColor3f(1.0, 0.2, 0);

			glBegin(GL_LINE_LOOP);
			for (int i = 0; i <= 300; i++) {
				double angle = 2 * M_PI * i / 300;
				double x = cos(angle)*0.08 * gTable.holes[j].radius + gTable.holes[j].centre(0);
				double y = sin(angle)*0.08 * gTable.holes[j].radius + gTable.holes[j].centre(1);
				glVertex3d(x, 0, y);
			}
			glEnd();

			glColor3f(1.0, 1.0, 1.0);
		}


		//draw the cue
		if (gDoCue)
		{
			glBegin(GL_LINES);
			float cuex = sin(gCueAngle) * gCuePower;
			float cuez = cos(gCueAngle) * gCuePower;
			int rat = player / 3 + 1;
			if (player % 3 == 0) {
				glColor3f(1.0/rat, 0.0, 0.0);
			}
			else if(player % 3 == 1)
			{
				glColor3f(0.0, 0.0, 1.0/rat);
			}
			else
			{
				glColor3f(0.0, 1.0/rat, 0.0);
			}
			glVertex3f(gTable.balls[player].position(0), (BALL_RADIUS / 2.0f), gTable.balls[player].position(1));
			glVertex3f((gTable.balls[player].position(0) + cuex), (BALL_RADIUS / 2.0f), (gTable.balls[player].position(1) + cuez));
			glColor3f(1.0, 1.0, 1.0);
			glEnd();
		}

		//Draw Scores Text
		char buffer[33];

		sprintf_s(buffer, "Hole #%d", gTable.holeNo);
		drawBitmapText(buffer, -0.8, 0.7, 0);

		//glColor3f(1.0, 0, 0);
		sprintf_s(buffer, "Player %d: %d", player + 1, gTable.balls[player].score);
		drawBitmapText(buffer, 0.8, 0.7, 0);
		sprintf_s(buffer, "Player %d: %d", (player + 1)%(NUM_BALLS) + 1, gTable.balls[(player+1)%NUM_BALLS].score);
		drawBitmapText(buffer, 0.8, 0.6, 0);

		glColor3f(1.0, 1.0, 1.0);


		//glPopMatrix();
	}

	glFlush();
	glutSwapBuffers();
}

void SpecKeyboardFunc(int key, int x, int y) 
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		{
			gCueControl[0] = true;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			gCueControl[1] = true;
			break;
		}
		case GLUT_KEY_UP:
		{
			gCueControl[2] = true;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			gCueControl[3] = true;
			break;
		}
	}
}

void SpecKeyboardUpFunc(int key, int x, int y) 
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		{
			gCueControl[0] = false;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			gCueControl[1] = false;
			break;
		}
		case GLUT_KEY_UP:
		{
			gCueControl[2] = false;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			gCueControl[3] = false;
			break;
		}
	}
}

void KeyboardFunc(unsigned char key, int x, int y) 
{
	switch(key)
	{
	case(13):
		{
			if (gMenu.drawMenu)
			{
				gMenu.drawMenu = false;
				gTable.SetupCushions();
				break;
			}
			if(gDoCue)
			{
				gTable.balls[player].score++;
				gTable.balls[player].isGhost = false;
				vec2 imp(	(-sin(gCueAngle) * gCuePower * gCueBallFactor),
							(-cos(gCueAngle) * gCuePower * gCueBallFactor));
				gTable.balls[player].ApplyImpulse(imp);
				player++;
			}

			bool isAnyballInPlay = false;
			for (int i = 0; i < NUM_BALLS; i++)
				if (gTable.balls[i].isInPlay)
					isAnyballInPlay = true;

			if (!isAnyballInPlay)
			{
				gTable.holeNo++;
				if (gTable.holeNo > 9)
					gMenu.drawMenu = true;
				gTable.ResetTable();
			}
			break;
		}
	case(27):
		{
			for(int i=0;i<NUM_BALLS;i++)
			{
				gTable.balls[i].Reset();
			}
			break;
		}
	case(32):
		{
			gCamRotate = false;
			break;
		}
	case('z'):
		{
			gCamL = true;
			break;
		}
	case('c'):
		{
			gCamR = true;
			break;
		}
	case('s'):
		{
			gCamU = true;
			break;
		}
	case('x'):
		{
			gCamD = true;
			break;
		}
	case('f'):
		{
			gCamZin = true;
			break;
		}
	case('v'):
		{
			gCamZout = true;
			break;
		}
	}

}

void KeyboardUpFunc(unsigned char key, int x, int y) 
{
	switch(key)
	{
	case(32):
		{
			gCamRotate = true;
			break;
		}
	case('z'):
		{
			gCamL = false;
			break;
		}
	case('c'):
		{
			gCamR = false;
			break;
		}
	case('s'):
		{
			gCamU = false;
			break;
		}
	case('x'):
		{
			gCamD = false;
			break;
		}
	case('f'):
		{
			gCamZin = false;
			break;
		}
	case('v'):
		{
			gCamZout = false;
			break;
		}
	}
}

void ChangeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0) h = 1;
	float ratio = 1.0* w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,0.2,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0,0.7,2.1, 0.0,0.0,0.0, 0.0f,1.0f,0.0f);
	gluLookAt(gCamPos(0),gCamPos(1),gCamPos(2),gCamLookAt(0),gCamLookAt(1),gCamLookAt(2),0.0f,1.0f,0.0f);
}

void InitLights(void)
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glClearColor (0.0, 0.3, 0.0, 1.0);
	glShadeModel (GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
}
void UpdateScene(int ms) 
{
	if(gTable.AnyBallsMoving()==false) gDoCue = true;
	else gDoCue = false;


	bool isAnyballInPlay = false;
	for (int i = 0; i < NUM_BALLS; i++)
		if (gTable.balls[i].isInPlay)
			isAnyballInPlay = true;

	if (!gTable.balls[player].isInPlay && isAnyballInPlay)
	{
		player++;
		if (player >= NUM_BALLS)
			player = 0;
	}

	if(gDoCue)
	{
		if(gCueControl[0]) gCueAngle -= ((gCueAngleSpeed * ms)/1000);
		if(gCueControl[1]) gCueAngle += ((gCueAngleSpeed * ms)/1000);
		if (gCueAngle <0.0) gCueAngle += TWO_PI;
		if (gCueAngle >TWO_PI) gCueAngle -= TWO_PI;

		gMenu.menuUpdateTimer += gMenu.getDeltaTime();

		if (gCueControl[2])
		{
			gCuePower += ((gCuePowerSpeed * ms) / 1000);
			if (gMenu.menuUpdateTimer > gMenu.menuUpdateRate && gMenu.drawMenu)
			{
				gMenu.menuUpdateTimer = 0;
				gMenu.menuSelection++;
			}
		}
		if (gCueControl[3])
		{
			gCuePower -= ((gCuePowerSpeed * ms) / 1000);
			if (gMenu.menuUpdateTimer > gMenu.menuUpdateRate && gMenu.drawMenu)
			{
				gMenu.menuUpdateTimer = 0;
				gMenu.menuSelection--;
			}
		}
		if(gCuePower > gCuePowerMax) gCuePower = gCuePowerMax;
		if(gCuePower < gCuePowerMin) gCuePower = gCuePowerMin;
		if (gMenu.menuSelection > gMenu.menuSelectionMax) gMenu.menuSelection = gMenu.menuSelectionMax;
		if (gMenu.menuSelection < gMenu.menuSelectionMin) gMenu.menuSelection = gMenu.menuSelectionMin;
	}

	DoCamera(ms);

	gTable.Update(ms);

	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutPostRedisplay();
}

int _tmain(int argc, _TCHAR* argv[])
{
	gTable.SetupCushions();

	gMenu.lastTimeCheck = clock();

	glutInit(&argc, ((char **)argv));
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE| GLUT_RGBA);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(1000,700);
	//glutFullScreen();
	glutCreateWindow("MSc Assignment : Golf Game");
	#if DRAW_SOLID
	InitLights();
	#endif
	glutDisplayFunc(RenderScene);
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutReshapeFunc(ChangeSize);
	glutIdleFunc(RenderScene);
	
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(KeyboardFunc);
	glutKeyboardUpFunc(KeyboardUpFunc);
	glutSpecialFunc(SpecKeyboardFunc);
	glutSpecialUpFunc(SpecKeyboardUpFunc);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}
