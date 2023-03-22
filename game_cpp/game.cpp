
#include <cassert>
#include <cmath>
#include <array>

#include "../framework/scene.hpp"
#include "../framework/game.hpp"


//-------------------------------------------------------
//	game parameters
//-------------------------------------------------------

namespace params
{
	namespace ship
	{
		constexpr float LINEAR_SPEED = 0.5f;
		constexpr float ANGULAR_SPEED = 0.5f;
	}

	namespace aircraft
	{
		constexpr float LINEAR_SPEED = 2.f;
		constexpr float ANGULAR_SPEED = 2.5f;
		constexpr float FLIGHT_TIME = 10.f;
		constexpr float REFUEL_TIME = 3.f;
	}

	constexpr float PI = 3.14159265358979f;
}


//-------------------------------------------------------
//	Basic Vector2 class
//-------------------------------------------------------

class Vector2
{
public:
	float x;
	float y;

	Vector2();
	Vector2( float vx, float vy );
	Vector2( Vector2 const &other );

	float length();
};


Vector2::Vector2() :
	x( 0.f ),
	y( 0.f )
{
}


Vector2::Vector2( float vx, float vy ) :
	x( vx ),
	y( vy )
{
}


Vector2::Vector2( Vector2 const &other ) :
	x( other.x ),
	y( other.y )
{
}


Vector2 operator + ( Vector2 const &left, Vector2 const &right )
{
	return Vector2( left.x + right.x, left.y + right.y );
}


Vector2 operator - ( Vector2 const& left, Vector2 const& right )
{
	return Vector2( left.x - right.x, left.y - right.y );
}


Vector2 operator * ( float left, Vector2 const &right )
{
	return Vector2( left * right.x, left * right.y );
}


float Vector2::length()
{
	return std::sqrt( x * x + y * y );
}


//-------------------------------------------------------
//	Aircraft
//-------------------------------------------------------

enum class AircraftState
{
	IDLE,
	TAKEOFF,
	FLY,
	HOVER,
	LAND,
	REFUEL
};


class Aircraft
{
public:
	Aircraft();

	void init( class Ship *owner );
	void deinit();
	void update( float dt );

	void setTarget( Vector2 const& position );
	void launch();
	bool readyToFly() const;
	bool inFlight() const;

protected:
	void takeoff( float dt );
	void fly( float dt );
	void hover( float dt );
	void land( float dt );
	void refuel( float dt );

	void simulateFlight( float dt );

private:
	scene::Mesh *mesh;
	Vector2 position;
	float angle;
	float acceleration;
	float linearSpeed;

	float takeoffTime;
	float flightTime;
	float landingTime;

	Vector2 targetPosition;
	float hoverRaduis;
	float hoverAngle;

	class Ship *owningShip;
	AircraftState state;
};


//-------------------------------------------------------
//	Ship
//-------------------------------------------------------

class Ship
{
public:
	Ship();

	void init( std::array< Aircraft, 5 > *aircrafts );
	void deinit();
	void update( float dt );
	void keyPressed( int key );
	void keyReleased( int key );
	void mouseClicked( Vector2 worldPosition, bool isLeftButton );

	Vector2 getPosition() const { return position; }
	float getAngle() const { return angle; }
	float getLinearSpeed() const { return linearSpeed; }

private:
	scene::Mesh *mesh;
	Vector2 position;
	float angle;
	float linearSpeed;

	bool input[ game::KEY_COUNT ];

	std::array< Aircraft, 5 > *planes;
};


//-------------------------------------------------------
//	Simple aircraft logic
//-------------------------------------------------------

Aircraft::Aircraft() :
	mesh( nullptr )
{
}


void Aircraft::init( Ship *owner )
{
	position = Vector2( 0.f, 0.f );
	angle = 0.f;
	acceleration = 1.f;
	linearSpeed = 0.f;

	takeoffTime = 2.f;
	flightTime = 0.f;
	landingTime = 0.f;

	hoverRaduis = 1.f;
	hoverAngle = 0.f;

	owningShip = owner;
	state = AircraftState::IDLE;
}


void Aircraft::deinit()
{
	scene::destroyMesh( mesh );
	mesh = nullptr;
}


void Aircraft::update( float dt )
{
	switch ( state )
	{
	case AircraftState::TAKEOFF:
		takeoff( dt );
		break;
	case AircraftState::FLY:
		fly( dt );
		break;
	case AircraftState::HOVER:
		hover( dt );
		break;
	case AircraftState::LAND:
		land( dt );
		break;
	case AircraftState::REFUEL:
		refuel( dt );
		break;
	default:
		break;
	}

	simulateFlight( dt );
}


void Aircraft::setTarget( Vector2 const& position )
{
	targetPosition = position;
}


bool Aircraft::readyToFly() const
{
	return state == AircraftState::IDLE;
}


bool Aircraft::inFlight() const
{
	return state != AircraftState::IDLE && state != AircraftState::REFUEL;
}


void Aircraft::launch()
{
	mesh = scene::createAircraftMesh();
	position = owningShip->getPosition();
	angle = owningShip->getAngle();
	scene::placeMesh( mesh, position.x, position.y, angle );

	state = AircraftState::TAKEOFF;
}


void Aircraft::takeoff( float dt )
{
	if ( flightTime >= takeoffTime )
		state = AircraftState::FLY;

	angle = owningShip->getAngle();
	float speed = linearSpeed + owningShip->getLinearSpeed();
	position = position + speed * dt * Vector2( std::cos(angle), std::sin(angle) );
}


void Aircraft::fly( float dt )
{
	float radiusToTarget = ( targetPosition - position ).length();
	if ( radiusToTarget <= hoverRaduis )
	{
		state = AircraftState::HOVER;
		hoverAngle = angle + params::PI;
		return;
	}

	angle = std::atan2( targetPosition.y - position.y, targetPosition.x - position.x );
	position = position + linearSpeed * dt * Vector2( std::cos(angle), std::sin(angle) );
}


void Aircraft::hover( float dt )
{
	float radiusToTarget = ( targetPosition - position ).length();
	float errorTolerance = 0.000001f;
	if ( radiusToTarget > hoverRaduis + errorTolerance )
	{
		state = AircraftState::FLY;
		return;
	}

	if ( flightTime >= params::aircraft::FLIGHT_TIME )
		state = AircraftState::LAND;

	angle = hoverAngle + params::PI / 2;
	hoverAngle = hoverAngle + params::aircraft::ANGULAR_SPEED * dt;
	float posX = targetPosition.x + std::cos( hoverAngle ) * hoverRaduis;
	float posY = targetPosition.y + std::sin( hoverAngle ) * hoverRaduis;
	position = Vector2( posX, posY );
}


void Aircraft::land( float dt )
{
	Vector2 landingPos = owningShip->getPosition();
	float distanceToShip = ( landingPos - position ).length();
	if ( distanceToShip <= 0.1f )
	{
		state = AircraftState::REFUEL;
		landingTime = flightTime;
		scene::destroyMesh( mesh );
	}

	angle = std::atan2( landingPos.y - position.y, landingPos.x - position.x );
	position = position + linearSpeed * dt * Vector2( std::cos(angle), std::sin(angle) );
}


void Aircraft::refuel( float dt )
{
	landingTime += dt;
	if ( landingTime > flightTime + params::aircraft::REFUEL_TIME )
	{
		state = AircraftState::IDLE;
		linearSpeed = 0.f;
		flightTime = 0.f;
		landingTime = 0.f;
	}
}


void Aircraft::simulateFlight( float dt )
{
	if ( !inFlight() ) return;

	float newSpeed = linearSpeed + acceleration * dt;
	float maxSpeed = params::aircraft::LINEAR_SPEED;
	if ( newSpeed <= maxSpeed )
		linearSpeed = newSpeed;
	else
		linearSpeed = maxSpeed;
	
	flightTime += dt;

	scene::placeMesh( mesh, position.x, position.y, angle );
}


//-------------------------------------------------------
//	Simple ship logic
//-------------------------------------------------------

Ship::Ship() :
	mesh( nullptr )
{
}


void Ship::init( std::array< Aircraft, 5 > *aircrafts )
{
	assert( !mesh );
	mesh = scene::createShipMesh();
	position = Vector2( 0.f, 0.f );
	angle = 0.f;
	for ( bool &key : input )
		key = false;

	planes = aircrafts;
}


void Ship::deinit()
{
	scene::destroyMesh( mesh );
	mesh = nullptr;
}


void Ship::update( float dt )
{
	linearSpeed = 0.f;
	float angularSpeed = 0.f;

	if ( input[ game::KEY_FORWARD ] )
	{
		linearSpeed = params::ship::LINEAR_SPEED;
	}
	else if ( input[ game::KEY_BACKWARD ] )
	{
		linearSpeed = -params::ship::LINEAR_SPEED;
	}

	if ( input[ game::KEY_LEFT ] && linearSpeed != 0.f )
	{
		angularSpeed = params::ship::ANGULAR_SPEED;
	}
	else if ( input[ game::KEY_RIGHT ] && linearSpeed != 0.f )
	{
		angularSpeed = -params::ship::ANGULAR_SPEED;
	}

	angle = angle + angularSpeed * dt;
	position = position + linearSpeed * dt * Vector2( std::cos( angle ), std::sin( angle ) );
	scene::placeMesh( mesh, position.x, position.y, angle );
}


void Ship::keyPressed( int key )
{
	assert( key >= 0 && key < game::KEY_COUNT );
	input[ key ] = true;
}


void Ship::keyReleased( int key )
{
	assert( key >= 0 && key < game::KEY_COUNT );
	input[ key ] = false;
}


void Ship::mouseClicked( Vector2 worldPosition, bool isLeftButton )
{
	if ( isLeftButton )
	{
		scene::placeGoalMarker( worldPosition.x, worldPosition.y );
		for ( Aircraft &plane : *planes )
			plane.setTarget( worldPosition );
	}
	else
	{
		for ( Aircraft &plane : *planes )
		{
			if ( plane.readyToFly() )
			{
				plane.launch();
				break;
			}
		}
	}
}


//-------------------------------------------------------
//	game public interface
//-------------------------------------------------------

namespace game
{
	Ship ship;
	std::array< Aircraft, 5 > planes;

	void init()
	{
		ship.init( &planes );
		for ( Aircraft &plane : planes )
			plane.init( &ship );
	}


	void deinit()
	{
		ship.deinit();
		for ( Aircraft &plane : planes )
			if ( plane.inFlight() )
				plane.deinit();
	}


	void update( float dt )
	{
		ship.update( dt );
		for ( Aircraft &plane : planes )
			plane.update( dt );
	}


	void keyPressed( int key )
	{
		ship.keyPressed( key );
	}


	void keyReleased( int key )
	{
		ship.keyReleased( key );
	}


	void mouseClicked( float x, float y, bool isLeftButton )
	{
		Vector2 worldPosition( x, y );
		scene::screenToWorld( &worldPosition.x, &worldPosition.y );
		ship.mouseClicked( worldPosition, isLeftButton );
	}
}

