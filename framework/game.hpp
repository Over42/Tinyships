

namespace game
{
	void init();
	void deinit();
	void update( float dt );

	enum
	{
		KEY_FORWARD,
		KEY_BACKWARD,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_COUNT
	};

	void keyPressed( int key );
	void keyReleased( int key );
	void mouseClicked( float x, float y, bool isLeftButton );
}

