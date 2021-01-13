#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
Separation::Separation(Flock& flock)
	:m_flock{ flock }
{

}

SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 toTarget = {};
	Elite::Vector2 sum = {};
	float distance = {};

	for (int i = 0; i < m_flock.GetNrOfNeighbors(); i++)
	{
		toTarget = m_flock.GetNeighbors()[i]->GetPosition() - pAgent->GetPosition();
		distance = Elite::Distance(m_flock.GetNeighbors()[i]->GetPosition(), pAgent->GetPosition());
		steering.LinearVelocity -= toTarget * (m_flock.GetNeighborHoodRadius() / distance) * (m_flock.GetNeighborHoodRadius() / distance);
	}
	steering.LinearVelocity /= float(m_flock.GetNrOfNeighbors());
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= deltaT * pAgent->GetMaxLinearSpeed();

	return steering;
}

//*******************
//COHESION (FLOCKING)
Cohesion::Cohesion(Flock& flock)
	:m_flock{ flock }
{

}

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	steering.LinearVelocity =  m_flock.GetAverageNeighborPos() - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * deltaT;

	return steering;
}

//*************************
//ALIGNMENT (FLOCKING)
Alignment::Alignment(Flock& flock)
	:m_flock{flock}
{

}

SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	steering.LinearVelocity = m_flock.GetAverageNeighborVelocity();

	return steering;
}
