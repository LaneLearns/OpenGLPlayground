#include "Application.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Application::Application(const int width, const int height, const std::string& title)
	: m_Width(width), m_Height(height), m_Title(title)
{
}

Application::~Application()
{
	if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
	if (m_VBO) glDeleteBuffers(1, &m_VBO);
	if (m_EBO) glDeleteBuffers(1, &m_EBO);
	if (m_Texture) glDeleteTextures(1, &m_Texture);

	if (m_Window)
	{
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}

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
	
	// setup GPU resources for triangle
	SetupTriangle();

	// Load a texture
	m_Texture = LoadTextureFromFile("assets/Paper_280S.jpg");
	if (m_Texture == 0)
	{
		std::cerr << "Failed to load texture." << std::endl;
		return false;
	}

	// Tell the shader to use texture unit 0 for uTexture
	glUseProgram(m_ShaderProgram);
	if (m_TextureUniformLocation != -1)
	{
		glUniform1i(m_TextureUniformLocation, 0); // Texture unit 0
	}

	glUseProgram(0);

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
	(void)DeltaTime;

	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the triangle
	glUseProgram(m_ShaderProgram);

	// compute time value
	float timeValue = static_cast<float>(glfwGetTime());
	float angle = timeValue; // Rotate based on time

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

	// set the uniform, but only if the location is valid
	if (m_TimeUniformLocation != -1)
	{
		glUniform1f(m_TimeUniformLocation, timeValue);
	}

	if (m_ModelUniformLocation != -1)
	{
		glUniformMatrix4fv(
			m_ModelUniformLocation, 
			1,
			GL_FALSE, 
			glm::value_ptr(model)
		);
	}

	// Bind texture to texture unit 0
	if (m_Texture != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Texture);
	}

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);

}

void Application::SetupTriangle()
{
	// 1. Vertex Data for a simple triangle (positions only)
	float vertices[] = {
		// x,      y,      z,     r,    g,    b,     u,    v
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // top-left
	};

	// 2. Generate and bind VAO and VBO
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Indices for 2 triangles using 4 vertices
	unsigned int indices[] = {
		0, 1, 2,  // first triangle
		0, 2, 3   // second triangle
	};

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// position attribute (location 0)
	glVertexAttribPointer(
		0,                  // Attribute location
		3,                  // Number of components (x, y, z)
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(float),	// Stride 8 floats per vertex
		(void*)0			// Offset: start of the vertex data
	);
	glEnableVertexAttribArray(0);

	// color attribute (location 1)
	glVertexAttribPointer(
		1,                                  // Attribute location
		3,                                  // Number of components (r, g, b)
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(float),                  // Stride 8 floats per vertex
		(void*)(3 * sizeof(float))          // Offset: after the first 3 floats (position)
	);
	glEnableVertexAttribArray(1);

	// texture coord attribute (location 2)
	glVertexAttribPointer(
		2,                                  // Attribute location
		2,                                  // Number of components (u, v)
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(float),                  // Stride 8 floats per vertex
		(void*)(6 * sizeof(float))          // Offset: after the first 6 floats (position + color)
	);
	glEnableVertexAttribArray(2);

	// Unbind for cleanliness
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 4, Create shaders (inline sources for now)

	const char* vertexShaderSource = R"(
		#version 450 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec3 aColor;
		layout(location = 2) in vec2 aTexCoord;

		out vec3 vColor; // to pass to fragment shader
		out vec2 vTexCoord; // to pass texture coord to fragment shader
		uniform mat4 uModel;

		void main()
		{
			gl_Position = uModel * vec4(aPos, 1.0);
			vColor = aColor; // pass color to fragment shader
			vTexCoord = aTexCoord; // pass texture coord to fragment shader
		}
	)";

	const char* fragmentShaderSource = R"(
		#version 450 core
		uniform float uTime;
		uniform sampler2D uTexture;
		in vec3 vColor; // from vertex shader
		in vec2 vTexCoord; // from vertex shader
		out vec4 FragColor;

		void main()
		{
			float factor = 0.5 + 0.5 * sin(uTime);
			vec4 texColor = texture(uTexture, vTexCoord);
			FragColor = vec4(vColor * factor, 1.0) * texColor;
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

	m_ModelUniformLocation = glGetUniformLocation(m_ShaderProgram, "uModel");
	if (m_ModelUniformLocation == -1)
	{
		std::cerr << "Failed to get uniform location for uModel" << std::endl;
	}

	m_TextureUniformLocation = glGetUniformLocation(m_ShaderProgram, "uTexture");
	if (m_TextureUniformLocation == -1)
	{
		std::cerr << "Failed to get uniform location for uTexture" << std::endl;
	}

	// Shaders are now linked into the program; we can delete the individual objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

GLuint Application::LoadTextureFromFile(const char* path)
{
	int width = 0;
	int height = 0;
	int nrChannels = 0;

	// Flip vertically so the image's origin matches OpenGL's texture coordinates
	stbi_set_unpremultiply_on_load(1);

	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cerr << "Failed to load texture image: " << path << std::endl;
		return 0;
	}

	GLenum format = GL_RGB;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters (wrapping and filtering)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload piexel data
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width,
		height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Cleanup CPU image data
	stbi_image_free(data);

	// Optional: unbind
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}
