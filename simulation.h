/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"
#include <time.h>

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X			(0.6f) 
#define TABLE_Z			(1.2f)
#define TABLE_UNIT		(0.6f)
#define TABLE_Y			(0.1f)
#define TABLE_OB		(-100000.0f)
#define HOLE_RADIUS		(0.075f)
#define BALL_RADIUS		(0.05f)
#define BALL_MASS		(0.1f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(10)
#define NUM_BALLS		(7)		
#define NUM_HOLES		(3)		
#define NUM_CUSHIONS	(8)		

/*-----------------------------------------------------------
  plane normals
  -----------------------------------------------------------*/
/*
extern vec2	gPlaneNormal_Left;
extern vec2	gPlaneNormal_Top;
extern vec2	gPlaneNormal_Right;
extern vec2	gPlaneNormal_Bottom;
*/


/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;
	int		length;

	void	MakeNormal(void);
	void	MakeCentre(void);
	void	MakeLength(void);
};

/*-----------------------------------------------------------
hole class
-----------------------------------------------------------*/
class hole
{
public:
	vec2	centre;
	float	radius;
	bool	isTarget = true;
	hole() : radius(HOLE_RADIUS) {}
};

/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;
	int		score = 0;
	bool	isInPlay = true;
	bool	isGhost = true;

	ball(): position(0.0), velocity(0.0), radius(BALL_RADIUS), 
		mass(BALL_MASS) {index = ballIndexCnt++; Reset();}
	
	void	Reset(void);
	void	ApplyImpulse(vec2 imp);
	void	ApplyFrictionForce(int ms);
	void	Update(int ms);

	void	DoPlaneCollision(const cushion &c);
	void	DoBallCollision(ball &b);
	void	DoHoleCollision(const hole &h);
	
	bool	HasHitPlane(const cushion &c) const;
	bool	HasHitBall(const ball &b) const;
	bool	HasHitHole(const hole &h) const;

	void	HitPlane(const cushion &c);
	void	HitBall(ball &b);
	void	HitHole(const hole &h);
};

/*-----------------------------------------------------------
  course class
  -----------------------------------------------------------*/
class course
{
public:
	ball	balls[NUM_BALLS];	
	cushion	cushions[NUM_CUSHIONS];
	hole	holes[NUM_HOLES];
	int		holeNo = 1;
	void	SetupCushions(void);
	void	ResetTable(void);
	void	Update(int ms);	
	bool	AnyBallsMoving(void) const;
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
extern course gCourse;



/*-----------------------------------------------------------
menu class
-----------------------------------------------------------*/
class menu
{
public:
	int menuSelection = 1;
	int menuSelectionMin = 1;
	int menuSelectionMax = 3;
	time_t menuUpdateRate = 250;
	time_t menuUpdateTimer = 0;
	time_t lastTimeCheck;
	bool drawMenu = true;
	bool drawScores = false;

	time_t GetDeltaTime()
	{
		time_t diff = clock() - lastTimeCheck;
		lastTimeCheck = clock();
		return diff;
	}
};
/*-----------------------------------------------------------
global table
-----------------------------------------------------------*/
extern menu gMenu;