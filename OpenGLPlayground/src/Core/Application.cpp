#include "Application.h"

#include <iostream>

Application::Application(const int width, const int height, const std::string& title)
	: m_Width(width), m_Height(height), m_Title(title)
{
}

Application::~Application()
{
	if (m_Window)
	{
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}

	// Safe to call even if not initialized; GLFW handles it.
	glfwTerminate();
}

bool Application::Initialize()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}

	// Request an OpenGL 4.5 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	if (!m_Window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(m_Window);

	// Load OpenGL functions using GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	// Set the initial viewport
	glViewport(0, 0, m_Width, m_Height);

	// Optional: Enable V-Sync
	glfwSwapInterval(1);

	m_LastFrameTime = glfwGetTime();

	return true;
}

int Application::Run()
{
	if (!m_Window)
	{
		std::cerr << "Application not initialized. Call Initialize() before Run()." << std::endl;
		return -1;
	}

	while (!glfwWindowShouldClose(m_Window))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - m_LastFrameTime);
		m_LastFrameTime = currentTime;

		ProcessInput();
		Render(deltaTime);

		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	return 0;
}

void Application::ProcessInput()
{
	if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_Window, true);
	}
}

void Application::Render(float DeltaTime)
{
	// For now, delta time is unused, but can be utilized for animations or time-based updates.

	DeltaTime++; // Prevent unused variable warning

	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}