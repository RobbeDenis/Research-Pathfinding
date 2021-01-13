//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0.f, 1.f, 0.f, 0.5f}, 0.40f);
	}

	return steering;
}


//FLEE
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	if (distanceToTarget > m_FleeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}

	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	steering.LinearVelocity = -1 * (m_Target.Position - pAgent->GetPosition());
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);
	float wanderAngle = Elite::randomFloat(-m_AngleChange, m_AngleChange) + m_PreviousAngle;
	Elite::Vector2 agentDirection = pAgent->GetDirection();;
	Elite::Vector2 randomTarget;
	Elite::Vector2 circleCenter = pAgent->GetPosition() + agentDirection * m_Offset;
	Elite::Vector2 target;

	m_PreviousAngle = wanderAngle;
	randomTarget = { m_Radius * std::cos(wanderAngle), m_Radius * std::sin(wanderAngle) };
	target = circleCenter + randomTarget;
	
	steering.LinearVelocity = target - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition() + agentDirection * m_Offset, m_Radius, { 0.f,1.f,0.f,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawPoint(target,5, { 0.f,1.f,1.f,0.5f });
		DEBUGRENDERER2D->DrawPoint(circleCenter, 5, { 1.f,1.f,1.f,0.5f });
	}

	return steering;
}

void Wander::SetWanderOffset(float offset)
{
	m_Offset = offset;
}

//ARIVE
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);
	const float slowRadius = 3.f;
	const float arrivalRadius = 1.f;

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float distance = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();

	if (distance < arrivalRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
		return steering;
	}

	if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	return steering;
}

//PURSUIT
//******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	Elite::Vector2 goal = m_Target.GetDirection().Normalize() * m_Target.LinearVelocity * distanceToTarget / pAgent->GetMaxLinearSpeed();
	goal += m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity = goal;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(goal + pAgent->GetPosition(), 6, { 1.f,0.f,0.f,1.f });
	}

	return steering;
}

//EVADE
//******
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	if (distanceToTarget > m_EvadeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}

	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	Elite::Vector2 goal = m_Target.GetDirection().Normalize() * m_Target.LinearVelocity * distanceToTarget / pAgent->GetMaxLinearSpeed();
	goal += m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity = -goal;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(goal + pAgent->GetPosition(), 6, { 0.f,1.f,0.f,1.f });
	}

	return steering;
}

void Evade::SetEvadeRadius(float radius)
{
	m_EvadeRadius = radius;
}

//FACE 
//******
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	pAgent->SetMaxAngularSpeed(5);
	pAgent->SetAutoOrient(false);
	Elite::Vector2 v = m_Target.Position - pAgent->GetPosition();
	float goal = atan2f(v.y, v.x) + ToRadians(90);
	float orientation = pAgent->GetOrientation();

	float a = goal - orientation;
	float b = goal - orientation + ToRadians(360);
	float c = goal - orientation - ToRadians(360);
	float smallest;

	// Looking for the smallest angle
	(std::abs(a) > std::abs(b)) ? (std::abs(b) > std::abs(c)) ? smallest = c : smallest = b : 
		(std::abs(a) > std::abs(c)) ? smallest = c : smallest = a;

	if (smallest < 0)
	{
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
	}
	else
	{
		steering.AngularVelocity = +pAgent->GetMaxAngularSpeed();
	}
	return steering;
}