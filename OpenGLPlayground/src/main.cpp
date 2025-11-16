#include "Application.h"

int main()
{
	Application app(1280, 720, "OpenGL Playground");
	
	if (!app.Initialize())
	{
		return -1;
	}

	return app.Run();
}