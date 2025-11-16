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

		// NEW: setup GPU resources for triangle
		SetupTriangle();

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
	(void)DeltaTime;

	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the triangle
	glUseProgram(m_ShaderProgram);
	glBindVertexArray(m_VAO);
	// compute time value
	float timeValue = static_cast<float>(glfwGetTime());

	// set the uniform, but only if the location is valid
	if (m_TimeUniformLocation != -1)
	{
		glUniform1f(m_TimeUniformLocation, timeValue);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glUseProgram(0);

}

void Application::SetupTriangle()
{
	// 1. Vertex Data for a simple triangle (positions only)
	float vertices[] = {
		// x,      y,      z,     r,    g,    b
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.0f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f  // top

	};

	// 2. Generate and bind VAO and VBO
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute (location 0)
	glVertexAttribPointer(
		0,                  // Attribute location
		3,                  // Number of components (x, y, z)
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),	// Stride 6 floats per vertex
		(void*)0			// Offset: start of the vertex data
	);
	glEnableVertexAttribArray(0);

	// color attribute (location 1)
	glVertexAttribPointer(
		1,                                  // Attribute location
		3,                                  // Number of components (r, g, b)
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),                  // Stride 6 floats per vertex
		(void*)(3 * sizeof(float))          // Offset: after the first 3 floats (position)
	);
	glEnableVertexAttribArray(1);

	// Unbind for cleanliness
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 4, Create shaders (inline sources for now)

	const char* vertexShaderSource = R"(
		#version 450 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec3 aColor;

		out vec3 vColor; // to pass to fragment shader

		void main()
		{
			gl_Position = vec4(aPos, 1.0);
			vColor = aColor; // pass color to fragment shader
		}
	)";

	const char* fragmentShaderSource = R"(
		#version 450 core
		uniform float uTime;
		in vec3 vColor; // from vertex shader
		out vec4 FragColor;

		void main()
		{
			float factor = 0.5 + 0.5 * sin(uTime);
			FragColor = vec4(vColor * factor, 1.0);
		}
	)";

	// Compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
	}

	// Compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
	}

	// Link shaders into a program
	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, vertexShader);
	glAttachShader(m_ShaderProgram, fragmentShader);
	glLinkProgram(m_ShaderProgram);

	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);

	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
		std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
	}

	// NEW: Get uniform location for time
	m_TimeUniformLocation = glGetUniformLocation(m_ShaderProgram, "uTime");
	if (m_TimeUniformLocation == -1)
	{
		std::cerr << "Failed to get uniform location for uTime" << std::endl;
	}

	// Shaders are now linked into the program; we can delete the individual objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
