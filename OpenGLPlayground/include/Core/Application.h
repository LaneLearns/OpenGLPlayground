#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application
{
public:
	Application(const int width, const int height, const std::string& title);
	~Application();

	// Initialize libraries, create window, load OpenGL.
	bool Initialize();

	// Main loop
	int Run();

private:

	void ProcessInput();
	void Render(float DeltaTime);

	int m_Width;
	int m_Height;
	std::string m_Title;

	GLFWwindow* m_Window = nullptr;

	double m_LastFrameTime = 0.0;
};