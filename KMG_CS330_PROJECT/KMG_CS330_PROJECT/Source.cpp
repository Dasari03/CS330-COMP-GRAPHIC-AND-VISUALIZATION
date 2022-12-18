#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <cmath>

using namespace std;

int width, height;
GLuint triangleVBO, planeVBO, verticesVBO, triangleVAO, planeVAO, verticesVAO;; // triangle and plane VBO and VAO variables
float camX, camY, camZ; // view or camera location (x,y,z)
float triLocX, triLocY, triLocZ; // triangle location (x,y,z)
float planeLocX, planeLocY, planeLocZ; // plane location (x,y,z)
float pLocX, pLocY, pLocZ; // plane location (x,y,z)
GLuint mvLoc, projLoc; // mvp uniform reference variables
float aspectRatio; // for perspective matrix aspect ratio
glm::mat4 pMat, vMat, mMat, mvMat; // mvp matrix variables
GLuint shaderProgram; // for compiled shaders

glm::float32 triRotations[] = { 0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f };
glm::float32 planeRotations[] = { 0.0f, 90.f, 0.0f, 90.f };

// mgk

// Draw Primitive(s)
void drawElement()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 6;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);


}




//mgk






/* GLSL Error Checking Definitions */
void PrintShaderCompileError(GLuint shader)
{
	int len = 0;
	int chWritten = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0)
	{
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWritten, log);
		cout << "Shader Compile Error: " << log << endl;
		free(log);
	}
}


void PrintShaderLinkingError(int prog)
{
	int len = 0;
	int chWritten = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0)
	{
		log = (char*)malloc(len);
		glGetShaderInfoLog(prog, len, &chWritten, log);
		cout << "Shader Linking Error: " << log << endl;
		free(log);
	}
}


bool IsOpenGLError()
{
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}



void drawCube() {
	// Activate VAO (now references planeVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(planeVAO);


	// Build Model Matrix for Plane
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(planeLocX, planeLocY, planeLocZ)); // translate plane (note initial glm::mat4(1.0f))	
	mMat = glm::rotate(mMat, glm::radians(45.f), glm::vec3(1.0f, -0.5f, 0.0f)); // concatenate rotatation (note mMat)
	//mMat = glm::scale(mMat, glm::vec3(5.f, 5.f, 5.f)); // Scale plane by 500%
	//mMat = glm::scale(mMat, glm::vec3(1.5f, 1.5f, 1.5f)); // reduce plane scale by 50%
	mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 40); // Render primitive or execute shader per draw	

	glBindVertexArray(0);// Optional unbinding but recommended
};

void drawPyramid() {
	// Activate VAO (now references planeVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(verticesVAO);


	// Build Model Matrix for Pyramid
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(pLocX, pLocY, pLocZ)); // translate plane (note initial glm::mat4(1.0f))	
	mMat = glm::rotate(mMat, glm::radians(45.f), glm::vec3(1.0f, -0.5f, 0.0f)); // concatenate rotatation (note mMat)
	//mMat = glm::scale(mMat, glm::vec3(2.5f, 1.f, 1.f)); // Scale plane by %
	//mMat = glm::scale(mMat, glm::vec3(1.5f, 1.5f, 1.5f)); // reduce plane scale by 50%
	mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 12); // Render primitive or execute shader per draw	

	glBindVertexArray(0);// Optional unbinding but recommended

};





/* GLSL Error Checking Definitions End Here */

/*Draw Primitive(s)*/
void draw(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT); // Z-buffer operation (hsr removal)
	glClear(GL_COLOR_BUFFER_BIT); // remove animation trails

	glUseProgram(shaderProgram); // load shaders to GPU

	// Reference matrix uniform variables in shader 
	mvLoc = glGetUniformLocation(shaderProgram, "mv_matrix");
	projLoc = glGetUniformLocation(shaderProgram, "proj_matrix");

	// Build Perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspectRatio, 0.1f, 1000.0f);

	//Build View matrix
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, -camZ));

	
	
	drawCube();
	drawPyramid();
}

// Set up view (camera), triangle, and plane locations
void initPositions()
{
	camX = 0.0f;
	camY = 0.0f;
	camZ = 10.0f;
	triLocX = 0.0f;
	triLocY = 0.0f;
	triLocZ = 0.0f; //0.0f
	
	pLocX = -0.2f; //-0.3f
	pLocY = 1.5f;
	pLocZ = 2.0f; //1.0f
}


/*Compile Shaders */
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create vertexShader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile shader
	glCompileShader(shaderID);


	/* Shader Compliation Error Check */
	GLint shaderCompiled;
	IsOpenGLError();
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != 1)
	{
		cout << "Shader Compilation Failed!" << endl;
		PrintShaderCompileError(shaderID);
	}
	/* End here */


	// Return ID
	return shaderID;
}

/* Create Shader Program*/
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile Vertex Shader
	GLint vertShaderCompiled = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile Fragment Shader
	GLint fragShaderCompiled = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach shaders
	glAttachShader(shaderProgram, vertShaderCompiled);
	glAttachShader(shaderProgram, fragShaderCompiled);

	// Link shaders to create full shader program
	glLinkProgram(shaderProgram);

	/* Shader Linking Error Check */
	GLint linked;
	IsOpenGLError();
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
	if (linked != 1)
	{
		cout << "Shader Linking Failed!" << endl;
		PrintShaderLinkingError(shaderProgram);
	}
	/* End here */

	glValidateProgram(shaderProgram);

	// Delete intermediates
	glDeleteShader(vertShaderCompiled);
	glDeleteShader(fragShaderCompiled);

	return shaderProgram;
}

void CreateVertices()
{
	/*
	// Define vertex data for Triangle (GLfloat array)
	GLfloat triangleVertices[] = {
		// positon attributes (x,y,z)
		0.0f, .0f, 0.0f,  // vert 1
		1.0f, 0.0f, 0.0f, // red

		0.5f, 0.866f, 0.0f, // vert 2
		0.0f, 1.0f, 0.0f, // green

		1.0f, 0.0f, 0.0f, // vert 3
		0.0f, 0.0f, 1.0f, // blue

		0.5f, 0.866f, 0.0f, // vert 4
		0.0f, 1.0f, 0.0f, // green

		0.5f, 0.866f, -2.0f, // vert 5
		0.0f, 1.0f, 0.0f, // green	

		1.0f, 0.0f, 0.0f, // vert 6
		0.0f, 0.0f, 1.0f, // blue

		1.0f, 0.0f, 0.0f, // vert 7
		0.0f, 0.0f, 1.0f, // blue

		1.0f, 0.0f, -2.0f, // vert 8
		0.0f, 0.0f, 1.0f, // blue

		0.5f, 0.866f, -2.0f, // vert 9
		0.0f, 1.0f, 0.0f // green
	};
	*/


	GLfloat planeVertices[] = {
		// positon attributes (x,y,z)
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,

		 1.0f, 1.0f, -1.0f,
		 0.0f, 1.1f, 1.1f,

		 1.0f, 1.0f, 1.0f,//12
		 1.0f, 1.0f, 1.0f,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,

		 1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 1.0f,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,//24

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,

		 -1.0f, 1.0f, -1.0f,
		  0.0f, 1.0f, 0.0f,

		 1.0f, 1.0f, -1.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		0.0f, 0.0f, 1.0f, //36

		   
};

	GLfloat vertices[] = {
		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,

		-1.0, -1.0, 1.0,
		0.0, 1.0, 0.0,

		1.0, -1.0, 1.0,
		0.0, 0.0, 1.0,

		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,

		1.0, -1.0, 1.0,
		0.0, 0.0, 1.0,

		1.0, -1.0, -1.0,
		0.0, 1.0, 0.0,

		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,

		1.0, -1.0, -1.0,
		0.0, 1.0, 0.0,

		-1.0, -1.0, -1.0,
		0.0, 0.0, 1.0,

		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,

		-1.0, -1.0, -1.0,
		0.0, 0.0, 1.0,

		-1.0, -1.0, 1.0,
		0.0, 1.0, 0.0

	};

/*
	glGenVertexArrays(1, &triangleVAO); // Create VAO
	glGenBuffers(1, &triangleVBO); // Create VBO
	glBindVertexArray(triangleVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)
*/

	glGenVertexArrays(1, &planeVAO); // Create VAO
	glGenBuffers(1, &planeVBO); // Create VBO
	glBindVertexArray(planeVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)

	//vertices
	glGenVertexArrays(1, &verticesVAO); // Create VAO
	glGenBuffers(1, &verticesVBO); // Create VBO
	glBindVertexArray(verticesVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)
}


int main(void)
{
	width = 1140; height = 580;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSwapInterval(1); // VSync operation


	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	initPositions();// Set up view (camera) and triangle initial positions
	CreateVertices(); // Generate vertices, create VAO, create buffers, associate VAO with VBO, load vertices to VBOs, associate VBOs with VAs (Vertex Attributes)

	// Write vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec4 vPosition;"
		"layout(location = 1) in vec4 aColor;"
		"out vec4 v_color;"
		"uniform mat4 mv_matrix;"
		"uniform mat4 proj_matrix;"
		"void main()\n"
		"{\n"
		"gl_Position = proj_matrix * mv_matrix * vPosition;"
		" v_color = aColor;"
		"}\n";

	// Write fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec4 v_color;"
		"out vec4 fragColor;"
		"uniform mat4 mv_matrix;"
		"uniform mat4 proj_matrix;"
		"void main()\n"
		"{\n"
		"fragColor = v_color;"
		"}\n";

	// Create shader (program object)
	shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);


		// Draw primitives
		draw(window, glfwGetTime());

		/* Swap front and back buffers */
		glfwSwapBuffers(window); // VSync operation

		/* Poll for and process events */
		glfwPollEvents(); // Detect keyboard and mouse input
	}

	glfwTerminate();
	return 0;
}








