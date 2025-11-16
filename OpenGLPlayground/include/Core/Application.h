#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application
{
public:
	Application(int width, int height, const std::string& title);
	~Application();

	// Initialize libraries, create window, load OpenGL.
	bool Initialize();

	// Main loop
	int Run();

private:

	void ProcessInput();
	void Render(float deltaTime);

	void SetupTriangle();

	int m_Width;
	int m_Height;
	std::string m_Title;

	GLFWwindow* m_Window = nullptr;

	double m_LastFrameTime = 0.0;

	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_ShaderProgram = 0;
	GLint m_TimeUniformLocation = -1;
};