#pragma once
#include "../SteeringBehaviors.h"

class Flock;

//SEPARATION - FLOCKING
//*********************
class Separation : public ISteeringBehavior
{
public:
	Separation(Flock& flock);
	virtual ~Separation() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent);

private:
	Flock& m_flock;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public ISteeringBehavior
{
public:
	Cohesion(Flock& flock);
	virtual ~Cohesion() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent);

private:
	Flock& m_flock;
};

//ALIGNMENT - FLOCKING
//************************
class Alignment : public ISteeringBehavior
{
public:
	Alignment(Flock& flock);
	virtual ~Alignment() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent);

private:
	Flock& m_flock;
};