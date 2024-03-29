/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include <iostream>
#include <ctime>

/*-----------------------------------------------------------
  macros
  -----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/
/*
vec2	gPlaneNormal_Left(1.0,0.0);
vec2	gPlaneNormal_Top(0.0,1.0);
vec2	gPlaneNormal_Right(-1.0,0.0);
vec2	gPlaneNormal_Bottom(0.0,-1.0);
*/

course gCourse;
menu gMenu;

static const float gRackPositionX[] = {0.0f,0.0f,(BALL_RADIUS*2.0f),(-BALL_RADIUS*2.0f),(BALL_RADIUS*4.0f)}; 
static const float gRackPositionZ[] = {0.5f,0.0f,(-BALL_RADIUS*3.0f),(-BALL_RADIUS*3.0f)}; 

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;


/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1]-vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre/=2.0;
}

void cushion::MakeLength(void)
{
	length = sqrt((vertices[0](0) - vertices[0](1))*(vertices[0](0) - vertices[0](1)) + (vertices[1](0) - vertices[1](1))*(vertices[1](0) - vertices[1](1)));
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

void ball::Reset(void)
{
	isInPlay = true;
	//set velocity to zero
	velocity = 0.0;
	std::cout << index;
	//work out rack position
	
	isGhost = true;

	position(1) = 1;
	position(0) = 0;
	return;
	
}

void ball::ApplyImpulse(vec2 imp)
{
	velocity = imp;
}

void ball::ApplyFrictionForce(int ms)
{
	if(velocity.Magnitude()<=0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms)/1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if(velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void ball::DoBallCollision(ball &b)
{
	if(HasHitBall(b)) HitBall(b);
}

void ball::DoPlaneCollision(const cushion &b)
{
	if(HasHitPlane(b)) HitPlane(b);
}

void ball::DoHoleCollision(const hole &h)
{
	if (HasHitHole(h)) HitHole(h);
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms)/1000.0f);
	//set small velocities to zero
	if(velocity.Magnitude()<SMALL_VELOCITY) velocity = 0.0;
}

bool ball::HasHitPlane(const cushion &c) const
{
	//if moving away from plane, cannot hit
	if(velocity.Dot(c.normal) >= 0.0) return false;
	
	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);

	bool isTop = c.vertices[0](1) > c.vertices[1](1);
	bool isLeft = c.vertices[0](0) > c.vertices[1](0);

	if (isTop && position(1) > c.vertices[0](1) + radius) return false;
	if (!isTop && position(1) > c.vertices[1](1) + radius) return false;
	if (isTop && position(1) < c.vertices[1](1) - radius) return false;
	if (!isTop && position(1) < c.vertices[0](1) - radius) return false;

	if (isLeft && position(0) > c.vertices[0](0) + radius) return false;
	if (!isLeft && position(0) > c.vertices[1](0) + radius) return false;
	if (isLeft && position(0) < c.vertices[1](0) - radius) return false;
	if (!isLeft && position(0) < c.vertices[0](0) - radius) return false;

	if(sep > radius ) return false;
	return true;
}

bool ball::HasHitBall(const ball &b) const
{
	if (isGhost || b.isGhost || !isInPlay || !b.isInPlay) return false;
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float) relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if(relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if(dist > (radius+b.radius)) return false;
	return true;
}

bool ball::HasHitHole(const hole &h) const
{
	//if in front of plane, then have not hit
	vec2 relPos = position - h.centre;
	double dist = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
	if (dist > h.radius) return false;
	return true;
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0+gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta; 

/*
	//assume elastic collision
	//find plane normal
	vec2 planeNorm = gPlaneNormal_Left;
	//split velocity into 2 components:
	//find velocity component perpendicular to plane
	vec2 perp = planeNorm*(velocity.Dot(planeNorm));
	//find velocity component parallel to plane
	vec2 parallel = velocity - perp;
	//reverse perpendicular component
	//parallel component is unchanged
	velocity = parallel + (-perp)*gCoeffRestitution;
*/
}

void ball::HitBall(ball &b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity-(relDir*perpV);
	vec2 parallelV2 = b.velocity-(relDir*perpV2);
	
	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV*(mass-b.mass))/sumMass) + (float)((perpV2*(2.0*b.mass))/sumMass);
	float perpVNew2 = (float)((perpV2*(b.mass-mass))/sumMass) + (float)((perpV*(2.0*mass))/sumMass);
	
	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir*perpVNew);
	b.velocity = parallelV2 + (relDir*perpVNew2);
}

void ball::HitHole(const hole &h)
{
	if (h.isTarget)
	{
		isInPlay = false;
		velocity = { 0,0 };
		position(0) = 10000;
	}
	else
	{
		Reset();
		score++;
	}
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
void course::SetupCushions(void)
{
	printf_s("%d", gMenu.menuSelection);
	if (gMenu.menuSelection == 1) 
	{
		cushions[0].vertices[0](0) = -TABLE_UNIT;
		cushions[0].vertices[0](1) = -TABLE_UNIT*2;
		cushions[0].vertices[1](0) = -TABLE_UNIT;
		cushions[0].vertices[1](1) = TABLE_UNIT*2;

		cushions[1].vertices[0](0) = -TABLE_UNIT;
		cushions[1].vertices[0](1) = TABLE_UNIT*2;
		cushions[1].vertices[1](0) = TABLE_UNIT;
		cushions[1].vertices[1](1) = TABLE_UNIT*2;

		cushions[2].vertices[0](0) = TABLE_UNIT;
		cushions[2].vertices[0](1) = TABLE_UNIT*2;
		cushions[2].vertices[1](0) = TABLE_UNIT;
		cushions[2].vertices[1](1) = -TABLE_UNIT*2;

		cushions[3].vertices[0](0) = TABLE_UNIT;
		cushions[3].vertices[0](1) = -TABLE_UNIT*2;
		cushions[3].vertices[1](0) = -TABLE_UNIT;
		cushions[3].vertices[1](1) = -TABLE_UNIT*2;

		cushions[4].vertices[0](0) = TABLE_OB;
		cushions[4].vertices[0](1) = TABLE_OB;
		cushions[4].vertices[1](0) = TABLE_OB;
		cushions[4].vertices[1](1) = TABLE_OB;

		cushions[5].vertices[0](0) = TABLE_OB;
		cushions[5].vertices[0](1) = TABLE_OB;
		cushions[5].vertices[1](0) = TABLE_OB;
		cushions[5].vertices[1](1) = TABLE_OB;

		cushions[6].vertices[0](0) = TABLE_OB;
		cushions[6].vertices[0](1) = TABLE_OB;
		cushions[6].vertices[1](0) = TABLE_OB;
		cushions[6].vertices[1](1) = TABLE_OB;

		cushions[7].vertices[0](0) = TABLE_OB;
		cushions[7].vertices[0](1) = TABLE_OB;
		cushions[7].vertices[1](0) = TABLE_OB;
		cushions[7].vertices[1](1) = TABLE_OB;
	}

	if (gMenu.menuSelection == 2)
	{
		cushions[0].vertices[0](0) = -TABLE_UNIT;
		cushions[0].vertices[0](1) = 0;
		cushions[0].vertices[1](0) = -TABLE_UNIT;
		cushions[0].vertices[1](1) = TABLE_UNIT * 2;

		cushions[1].vertices[0](0) = -TABLE_UNIT;
		cushions[1].vertices[0](1) = TABLE_UNIT * 2;
		cushions[1].vertices[1](0) = TABLE_UNIT;
		cushions[1].vertices[1](1) = TABLE_UNIT * 2;

		cushions[2].vertices[0](0) = TABLE_UNIT;
		cushions[2].vertices[0](1) = TABLE_UNIT * 2;
		cushions[2].vertices[1](0) = TABLE_UNIT;
		cushions[2].vertices[1](1) = -TABLE_UNIT * 2;

		cushions[3].vertices[0](0) = TABLE_UNIT;
		cushions[3].vertices[0](1) = -TABLE_UNIT * 2;
		cushions[3].vertices[1](0) = -TABLE_UNIT * 3;
		cushions[3].vertices[1](1) = -TABLE_UNIT * 2;

		cushions[4].vertices[1](0) = -TABLE_UNIT * 3;
		cushions[4].vertices[1](1) = 0;
		cushions[4].vertices[0](0) = -TABLE_UNIT * 3;
		cushions[4].vertices[0](1) = -TABLE_UNIT * 2;

		cushions[5].vertices[0](0) = -TABLE_UNIT * 3;
		cushions[5].vertices[0](1) = 0;
		cushions[5].vertices[1](0) = -TABLE_UNIT;
		cushions[5].vertices[1](1) = 0;

		cushions[6].vertices[0](0) = TABLE_OB;
		cushions[6].vertices[0](1) = TABLE_OB;
		cushions[6].vertices[1](0) = TABLE_OB;
		cushions[6].vertices[1](1) = TABLE_OB;

		cushions[7].vertices[0](0) = TABLE_OB;
		cushions[7].vertices[0](1) = TABLE_OB;
		cushions[7].vertices[1](0) = TABLE_OB;
		cushions[7].vertices[1](1) = TABLE_OB;
	}

	if (gMenu.menuSelection == 3)
	{
		cushions[0].vertices[0](0) = -TABLE_UNIT;
		cushions[0].vertices[0](1) = 0;
		cushions[0].vertices[1](0) = -TABLE_UNIT;
		cushions[0].vertices[1](1) = TABLE_UNIT * 2;

		cushions[1].vertices[0](0) = -TABLE_UNIT;
		cushions[1].vertices[0](1) = TABLE_UNIT * 2;
		cushions[1].vertices[1](0) = TABLE_UNIT;
		cushions[1].vertices[1](1) = TABLE_UNIT * 2;

		cushions[2].vertices[0](0) = TABLE_UNIT;
		cushions[2].vertices[0](1) = TABLE_UNIT * 2;
		cushions[2].vertices[1](0) = TABLE_UNIT;
		cushions[2].vertices[1](1) = -TABLE_UNIT * 2;

		cushions[3].vertices[0](0) = TABLE_UNIT;
		cushions[3].vertices[0](1) = -TABLE_UNIT * 2;
		cushions[3].vertices[1](0) = -TABLE_UNIT;
		cushions[3].vertices[1](1) = -TABLE_UNIT * 2;

		cushions[4].vertices[1](0) = -TABLE_UNIT * 3;
		cushions[4].vertices[1](1) = 0;
		cushions[4].vertices[0](0) = -TABLE_UNIT * 3;
		cushions[4].vertices[0](1) = -TABLE_UNIT * 4;

		cushions[5].vertices[0](0) = -TABLE_UNIT * 3;
		cushions[5].vertices[0](1) = 0;
		cushions[5].vertices[1](0) = -TABLE_UNIT;
		cushions[5].vertices[1](1) = 0;

		cushions[6].vertices[0](0) = -TABLE_UNIT;
		cushions[6].vertices[0](1) = -TABLE_UNIT * 4;
		cushions[6].vertices[1](0) = -TABLE_UNIT*3;
		cushions[6].vertices[1](1) = -TABLE_UNIT * 4;

		cushions[7].vertices[1](0) = -TABLE_UNIT;
		cushions[7].vertices[1](1) = -TABLE_UNIT * 4;
		cushions[7].vertices[0](0) = -TABLE_UNIT;
		cushions[7].vertices[0](1) = -TABLE_UNIT * 2;
	}

	for(int i=0;i<NUM_CUSHIONS;i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
		cushions[i].MakeLength();
	}
	srand(time(NULL));

	holeNo = 1;
	holes[0].radius = HOLE_RADIUS;

	for (int i = 1; i<NUM_HOLES; i++)
	{
		holes[i].isTarget = false;
		holes[i].radius = HOLE_RADIUS;
	}

	for (int i = 1; i<NUM_BALLS; i++)
	{
		balls[i].score = 0;
	}

	ResetTable();
}


void course::ResetTable(void)
{
	double x = rand();
	double y = rand();

	holes[0].centre = { ((x / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS), ((y / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS) };
	
	if(gMenu.menuSelection == 1)
		holes[0].centre = { holes[0].centre(0) , holes[0].centre(1) - TABLE_UNIT };
	if (gMenu.menuSelection == 2)
		holes[0].centre = { holes[0].centre(0) - (TABLE_UNIT * 2), holes[0].centre(1) - TABLE_UNIT };
	if (gMenu.menuSelection == 3)
		holes[0].centre = { holes[0].centre(0) - (TABLE_UNIT * 2) , holes[0].centre(1) - (TABLE_UNIT*3) };

	if (holeNo < 4)
	{
		holes[1].centre = {TABLE_OB, TABLE_OB};
		holes[2].centre = {TABLE_OB, TABLE_OB};
	}
	else if (holeNo < 7)
	{
		vec2 relPos = holes[1].centre - holes[0].centre;
		double dist = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
		do
		{
			x = rand();
			y = rand();
			holes[1].centre = { ((x / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS), ((y / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS) };
			holes[1].centre = { holes[1].centre(0) , holes[1].centre(1) - TABLE_UNIT };

			relPos = holes[1].centre - holes[0].centre;
			dist = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
		} while (dist < HOLE_RADIUS * 2);

		holes[2].centre = { TABLE_OB, TABLE_OB };
	}
	else
	{

		vec2 relPos = holes[1].centre - holes[0].centre;
		double dist = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
		do
		{
			x = rand();
			y = rand();
			holes[1].centre = { ((x / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS), ((y / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS) };
			holes[1].centre = { holes[1].centre(0) , holes[1].centre(1) - TABLE_UNIT };

			relPos = holes[1].centre - holes[0].centre;
			dist = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
		} while (dist < HOLE_RADIUS * 2);


		double dist1 = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
		do
		{
			x = rand();
			y = rand();
			holes[2].centre = { ((x / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS), ((y / (RAND_MAX)) * (TABLE_UNIT - HOLE_RADIUS) * 2) - (TABLE_UNIT - HOLE_RADIUS) };

			if (gMenu.menuSelection == 1)
				holes[2].centre = { holes[2].centre(0) , holes[2].centre(1) - TABLE_UNIT };
			else
				holes[2].centre = { holes[2].centre(0) - (TABLE_UNIT * 2), holes[2].centre(1) - TABLE_UNIT };

			relPos = holes[2].centre - holes[0].centre;
			dist = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
			relPos = holes[2].centre - holes[1].centre;
			dist1 = sqrt(relPos(0)*relPos(0) + (relPos(1)*relPos(1)));
		} while (dist < HOLE_RADIUS * 2 && dist1 < HOLE_RADIUS * 2);
	}

	for (int i = 0; i < NUM_BALLS; i++)
		balls[i].Reset();
}

void course::Update(int ms)
{
	//check for collisions for each ball
	for(int i=0;i<NUM_BALLS;i++) 
	{
		for(int j=0;j<NUM_CUSHIONS;j++)
		{
			balls[i].DoPlaneCollision(cushions[j]);
		}

		for(int j=(i+1);j<NUM_BALLS;j++) 
		{
			balls[i].DoBallCollision(balls[j]);
		}

		for (int j = 0; j<NUM_HOLES; j++)
		{
			balls[i].DoHoleCollision(holes[j]);
		}
	}
	
	//update all balls
	for(int i=0;i<NUM_BALLS;i++) balls[i].Update(ms);
}

bool course::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for(int i=0;i<NUM_BALLS;i++) 
	{
		if(balls[i].velocity(0)!=0.0) return true;
		if(balls[i].velocity(1)!=0.0) return true;
	}
	return false;
}
