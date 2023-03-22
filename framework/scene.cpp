
#include <windows.h>
#include <GL/gl.h>

#include <cassert>
#include <vector>
#include <algorithm>
#include <random>

#include "scene.hpp"


namespace scene
{
	constexpr float VIEW_WIDTH = 18.f;
	constexpr float VIEW_HEIGHT = 13.5f;
}


//-------------------------------------------------------
//	simple particles support
//-------------------------------------------------------

namespace
{
	struct Color
	{
		float r, g, b;
	};


	struct Particle
	{
		float x;
		float y;
		float life;
		Color color;
	};


	std::vector< Particle > particles;


	void addParticle( float x, float y, float life, Color color )
	{
		Particle particle = { x, y, life, color };
		particles.push_back( particle );
	}


	void updateParticles( float dt )
	{
		for ( Particle &particle : particles )
			particle.life -= dt;
		auto newEnd = std::remove_if( particles.begin(), particles.end(), []( Particle &particle ){ return particle.life <= 0.f; } );
		particles.erase( newEnd, particles.end() );
	}


	void drawParticles()
	{
		glLoadIdentity();
		glPointSize( 2.f );
		glBegin( GL_POINTS );
		for ( Particle const &particle : particles )
		{
			glColor3f( particle.color.r, particle.color.g, particle.color.b );
			glVertex2f( particle.x, particle.y );
		}
		glEnd();
	}
}


//-------------------------------------------------------
//	user interface: common mesh support
//-------------------------------------------------------

namespace scene
{
	class Mesh
	{
	public:
		float positionX = 0.f;
		float positionY = 0.f;
		float angle = 0.f;

		virtual ~Mesh();
		virtual void draw();
		virtual void update( float dt );

		static std::vector< Mesh* > meshes;
	};


	//-------------------------------------------------------
	std::vector< Mesh* > Mesh::meshes;


	//-------------------------------------------------------
	Mesh::~Mesh()
	{
	}


	//-------------------------------------------------------
	void Mesh::draw()
	{
		glLoadIdentity();
		glTranslatef( positionX, positionY, 0.f );
		glRotatef( angle * 180.f / 3.14159265f, 0.f, 0.f, 1.f );
	}


	//-------------------------------------------------------
	void Mesh::update( float dt )
	{
	}


	//-------------------------------------------------------
	template< class MeshClass >
	Mesh *createMesh()
	{
		Mesh *mesh = new MeshClass;
		Mesh::meshes.push_back( mesh );
		return mesh;
	}


	//-------------------------------------------------------
	void destroyMesh( Mesh *mesh )
	{
		auto it = std::find( Mesh::meshes.begin(), Mesh::meshes.end(), mesh );
		assert( it != Mesh::meshes.end() );
		Mesh::meshes.erase( it );
		delete mesh;
	}


	//-------------------------------------------------------
	void placeMesh( Mesh *mesh, float x, float y, float angle )
	{
		mesh->positionX = x;
		mesh->positionY = y;
		mesh->angle = angle;
	}
}


//-------------------------------------------------------
//	user interface: ShipMesh support
//-------------------------------------------------------

namespace
{
	class ShipMesh : public scene::Mesh
	{
	public:
		void draw() override;
	};


	//-------------------------------------------------------
	void ShipMesh::draw()
	{
		Mesh::draw();

		glRotatef( -90.f, 0.f, 0.f, 1.f );
		glScalef( 0.8f, 0.8f, 0.8f );

		glBegin( GL_TRIANGLES );
		glColor3f( 0.1f, 0.3f, 0.6f );

		glVertex2f( -0.1f, -0.4f );
		glVertex2f( 0.1f, -0.4f );
		glVertex2f( 0.1f, 0.4f );

		glVertex2f( -0.1f, 0.4f );
		glVertex2f( 0.1f, 0.4f );
		glVertex2f( -0.1f, -0.4f );

		glVertex2f( -0.1f, -0.4f );
		glVertex2f( -0.1f, 0.4f );
		glVertex2f( -0.15f, -0.1f );

		glVertex2f( 0.1f, -0.4f );
		glVertex2f( 0.1f, 0.4f );
		glVertex2f( 0.15f, -0.1f );

		glEnd();

		glLineWidth( 2.f );
		glBegin( GL_LINE_LOOP );
		glColor3f( 0.4f, 0.8f, 1.f );
		glVertex2f( -0.1f, -0.4f );
		glVertex2f( 0.1f, -0.4f );
		glVertex2f( 0.15f, -0.1f );
		glVertex2f( 0.1f, 0.4f );
		glVertex2f( -0.1f, 0.4f );
		glVertex2f( -0.15f, -0.1f );
		glEnd();
	}
}


namespace scene
{
	//-------------------------------------------------------
	Mesh *createShipMesh()
	{
		return createMesh< ShipMesh >();
	}
}


//-------------------------------------------------------
//	user interface: AircraftMesh support
//-------------------------------------------------------

namespace
{
	class AircraftMesh : public scene::Mesh
	{
	public:
		void draw() override;
		void update( float dt ) override;

	private:
		float nextParticleTimeout = 0.f;
	};


	//-------------------------------------------------------
	void AircraftMesh::draw()
	{
		Mesh::draw();

		glRotatef( -90.f, 0.f, 0.f, 1.f );

		glBegin( GL_TRIANGLES );
		glColor3f( 0.5f, 0.6f, 0.1f );
		glVertex2f( -0.06f, -0.1f );
		glVertex2f( 0.06f, -0.1f );
		glVertex2f( 0.f, 0.1f );
		glVertex2f( -0.1f, -0.1f );
		glVertex2f( 0.1f, -0.1f );
		glVertex2f( 0.f, 0.0f );
		glEnd();

		glLineWidth( 2.f );
		glBegin( GL_LINE_LOOP );
		glColor3f( 0.8f, 1.f, 0.2f );
		glVertex2f( -0.1f, -0.1f );
		glVertex2f( 0.1f, -0.1f );
		glVertex2f( 0.04f, -0.04f );
		glVertex2f( 0.f, 0.1f );
		glVertex2f( -0.04f, -0.04f );
		glEnd();
	}


	void AircraftMesh::update( float dt )
	{
		nextParticleTimeout -= dt;
		if ( nextParticleTimeout <= 0.f )
		{
			nextParticleTimeout += 0.1f;
			addParticle( positionX, positionY, 0.8f, Color{ 1.f, 1.f, 1.f } );
		}
	}
}

namespace scene
{
	//-------------------------------------------------------
	Mesh *createAircraftMesh()
	{
		return createMesh< AircraftMesh >();
	}
}


//-------------------------------------------------------
//	user interface: goal marker support
//-------------------------------------------------------

namespace
{
	struct
	{
		float x;
		float y;
	} goalMarker;


	void drawGoalMarker()
	{
		glLoadIdentity();
		glLineWidth( 3.f );
		glBegin( GL_LINES );
		glColor3f( 1.0f, 0.3f, 0.2f );
		glVertex2f( goalMarker.x - 0.1f, goalMarker.y - 0.1f );
		glVertex2f( goalMarker.x + 0.1f, goalMarker.y + 0.1f );
		glVertex2f( goalMarker.x - 0.1f, goalMarker.y + 0.1f );
		glVertex2f( goalMarker.x + 0.1f, goalMarker.y - 0.1f );
		glEnd();
	}
}


namespace scene
{
	void placeGoalMarker( float x, float y )
	{
		goalMarker.x = x;
		goalMarker.y = y;
	}
}


//-------------------------------------------------------
//	user interface: utility functions
//-------------------------------------------------------

namespace scene
{
	void screenToWorld( float *x, float *y )
	{
		*x = 0.5f * VIEW_WIDTH * ( 2.f * *x - 1.f );
		*y = 0.5f * VIEW_HEIGHT * ( 2.f * *y - 1.f );
	}
}


//-------------------------------------------------------
//	engine only interface
//-------------------------------------------------------

namespace scene
{
	namespace
	{
		constexpr float TIME_BETWEEN_SEA_PARTICLES = 0.02f;
		float timeToNextSeaParticle = 0.f;
		std::default_random_engine seaParticlesRandomEngine( 42 );
		std::uniform_real_distribution< float > seaParticlesHorizDistr( -0.5f * VIEW_WIDTH, 0.5f * VIEW_WIDTH );
		std::uniform_real_distribution< float > seaParticlesVertDistr( -0.5f * VIEW_HEIGHT, 0.5f * VIEW_HEIGHT );
	}


	void update( float dt )
	{
		for ( Mesh *mesh : Mesh::meshes )
			mesh->update( dt );
		updateParticles( dt );

		timeToNextSeaParticle += dt;
		while ( timeToNextSeaParticle > 0.f )
		{
			timeToNextSeaParticle -= TIME_BETWEEN_SEA_PARTICLES;
			addParticle( seaParticlesHorizDistr( seaParticlesRandomEngine ),
						 seaParticlesVertDistr( seaParticlesRandomEngine ),
						 3.f,
						 Color{ 0.15f, 0.3f, 0.6f } );
		}
	}


	void draw()
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glScalef( 2.f / VIEW_WIDTH, 2.f / VIEW_HEIGHT, 0.f );

		glDisable( GL_CULL_FACE );
		glClearColor( 0.1f, 0.2f, 0.4f, 0.f );
		glClear( GL_COLOR_BUFFER_BIT );
		glMatrixMode( GL_MODELVIEW );

		drawParticles();
		for ( Mesh *mesh : Mesh::meshes )
			mesh->draw();
		drawGoalMarker();
	}
}
