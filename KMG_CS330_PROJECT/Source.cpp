#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions



#include <vector>

using namespace std;

int width, height;
GLuint cubeVBO, planeVBO, verticesVBO, cubeVAO, planeVAO, verticesVAO, lampVAO, lampVBO, cylinderVAO, cylinderVBO, cube2VAO, cube2VBO, ballVAO, ballVBO; // triangle and plane VBO and VAO variables
float camX, camY, camZ; // view or camera location (x,y,z)
float planeLocX, planeLocY, planeLocZ; // triangle location (x,y,z)
float cubeLocX, cubeLocY, cubeLocZ; // plane location (x,y,z)
float pLocX, pLocY, pLocZ; // plane location (x,y,z)
float triLocX, triLocY, triLocZ; // triangle location (x,y,z)
//float planeLocX, planeLocY, planeLocZ; // plane location (x,y,z)
GLuint mvLoc, projLoc, lampMvLoc, lampProjLoc; // mvp uniform reference variables
float aspectRatio; // for perspective matrix aspect ratio
glm::mat4 pMat, vMat, mMat, mvMat; // mvp matrix variables
GLuint shaderProgram, lampShaderProgram; // for compiled shaders
bool isOrtho = false;
GLuint gTextureIdCrate;
GLuint gTextureIdGrid;
bool gIsCrate = true;
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

glm::float32 triRotations[] = { 0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f };
glm::float32 triRotations2[] = { 0.0f, 45.0f, 90.0f, 135.0f, 180.0f, 225.0f, 270.0f, 315.0f };

glm::float32 planeRotations[] = { 0.0f, 90.f, 0.0f, 90.f };



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum  Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position += Up * velocity;
		if (direction == DOWN)
			Position -= Up * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};


// camera
Camera gCamera(glm::vec3(0.0f, 0.0f, 10.0f)); //3.0F Z



// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}
















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

//*********************************module4*******

//Input Function prototypes
void key_callback(GLFWwindow* windwo, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

//Declare view matrix;
//glm::mat4 viewMatrix;

//Initialize FOV
GLfloat fov = 45.f;

//Define Camera Attributes
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

//Decalre target prototype
glm::vec3 getTarget();

// Camera transformation prototype
void TransformCamera();

//Boolean for keys and mouse buttons
bool keys[1024], mouseButtons[3];

//Boolean to check camera transformations
bool isPanning = false, isOrbiting = false;

//Radius, Pitch, Yaw
GLfloat radius = 3.f, rawYaw = 0.f, rawPitch = 0.f, degYaw, degPitch;

GLfloat deltaTime = 0.f, lastFrame = 0.f;
GLfloat lastX = 320, lastY = 240, xChange, yChange;

bool firstMouseMove = true; // Detect initial mouse movemnet

void initCamera();




//****************************************************module4 above

void drawBall() {
	// Activate VAO (now references triangleVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(ballVAO);

	for (int i = 0; i < 9999; i++) {

		// Build Model Matrix for Triangle
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.0, -3.0, 0.0)); // translate triangle (note initial glm::mat4(1.0f))	 
		mMat = glm::rotate(mMat, glm::radians(70.f), glm::vec3(1.0f, 0.0f, 0.0f)); // concatenate rotatation (note mMat)	
		mMat = glm::rotate(mMat, glm::radians(triRotations2[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // concatenate rotatation (note mMat)
		mMat = glm::scale(mMat, glm::vec3(1.f, 1.f, 1.f)); // Scale triangle
		mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

		//Copy perspective and MV matrices to uniforms
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

		glEnable(GL_DEPTH_TEST); // Z-buffer operation
		glDepthFunc(GL_LEQUAL); // Used with Depth test
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
		glDrawArrays(GL_TRIANGLES, 0, 9); // Render primitive or execute shader per draw	
	};
	glBindVertexArray(0);// Optional unbinding but recommended



};


void drawCylinder() {
	// Activate VAO (now references triangleVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(cylinderVAO);

	for (int i = 0; i < 9999; i++) {

		// Build Model Matrix for Triangle
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5, -0.7, 5.0)); // translate triangle (note initial glm::mat4(1.0f))	 
		mMat = glm::rotate(mMat, glm::radians(-55.f), glm::vec3(1.0f, 0.0f, 0.0f)); // concatenate rotatation (note mMat)	
		mMat = glm::rotate(mMat, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // concatenate rotatation (note mMat)
		mMat = glm::scale(mMat, glm::vec3(1.f, 1.f, 1.f)); // Scale triangle
		mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

		//Copy perspective and MV matrices to uniforms
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

		glEnable(GL_DEPTH_TEST); // Z-buffer operation
		glDepthFunc(GL_LEQUAL); // Used with Depth test
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
		glDrawArrays(GL_TRIANGLES, 0, 9); // Render primitive or execute shader per draw	
	};
	glBindVertexArray(0);// Optional unbinding but recommended



};

void drawPlane() {
	// Activate VAO (now references planeVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(planeVAO);


	// Build Model Matrix for Plane
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(planeLocX, planeLocY, planeLocZ)); // translate plane (note initial glm::mat4(1.0f))	
	mMat = glm::rotate(mMat, glm::radians(45.f), glm::vec3(-5.0f, -.5f, 0.0f)); // concatenate rotatation (note mMat)
	mMat = glm::scale(mMat, glm::vec3(3.f, 3.f, 3.f)); // Scale plane by 500%
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


void drawCube() {
	// Activate VAO (now references planeVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(cubeVAO);


	// Build Model Matrix for Plane
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(-4.0, 1.0, 0.0)); // translate plane (note initial glm::mat4(1.0f))	
	mMat = glm::rotate(mMat, glm::radians(50.f), glm::vec3(1.0f, -0.5f, 0.0f)); // concatenate rotatation (note mMat)
	mMat = glm::scale(mMat, glm::vec3(1.2f, 1.2f, 1.2f)); // Scale triangle

	mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

	//newly added
	pMat = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

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
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(-4.5, 3., 1.5)); // translate plane (note initial glm::mat4(1.0f))	
	mMat = glm::rotate(mMat, glm::radians(50.f), glm::vec3(1.0f, -0.5f, 0.0f)); // concatenate rotatation (note mMat)
	mMat = glm::scale(mMat, glm::vec3(1.5f, 1.5f, 1.5f)); // Scale triangle

	//newly added below
	mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

	pMat = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 12); // Render primitive or execute shader per draw	

	glBindVertexArray(0);// Optional unbinding but recommended

};

void drawCube2() {
	// Activate VAO (now references planeVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(cube2VAO);


	// Build Model Matrix for Plane
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(2.0, 4.0, -5.0)); // translate plane (note initial glm::mat4(1.0f))	
	mMat = glm::rotate(mMat, glm::radians(45.f), glm::vec3(1.0f, -0.5f, 0.0f)); // concatenate rotatation (note mMat)
	mMat = glm::scale(mMat, glm::vec3(3.f, 3.f, 3.f)); // Scale triangle

	mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

	//newly added
	pMat = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 40); // Render primitive or execute shader per draw	

	glBindVertexArray(0);// Optional unbinding but recommended
};




void drawLight() {
	// Activate VAO (now references planeVBO and VA association) Can be placed anywhere before glDrawArrays()
	glBindVertexArray(lampVAO);


	// Build Model Matrix for Plane
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f)); // translate plane (note initial glm::mat4(1.0f))	  //*lightPosition ?!?!?!?!?!?!?!?
	mMat = glm::rotate(mMat, glm::radians(45.f), glm::vec3(1.0f, -0.5f, 0.0f)); // concatenate rotatation (note mMat)
	mMat = glm::scale(mMat, glm::vec3(.5f, .5f, .5f)); // reduce plane scale by 50%

	mvMat = vMat * mMat; // Concatenate View matrix with Model matrix here (CPU) than in shader (GPU) for better performance

	//newly added
	pMat = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(lampMvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 40); // Render primitive or execute shader per draw	

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

	//get light and object color location
	GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

	//assign light and object colors
	glUniform3f(objectColorLoc, 0.56f, 0.26f, 0.65f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightColorLoc, 1.0, 1.0, 1.0);
	glUniform3f(viewPosLoc, camX, camY, camZ);




	// Build Perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;


	/*
	pMat = glm::perspective(1.0472f, aspectRatio, 0.1f, 1000.0f);

	//Build View matrix
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, -camZ));

	mvMat = glm::lookAt(cameraPosition, getTarget(), worldUp);

	*/


	//pMat = glm::perspective(1.0472f, aspectRatio, 0.1f, 1000.0f);
	pMat = glm::perspective(glm::radians(gCamera.Zoom), aspectRatio, 0.1f, 100.0f);  //glm::radians(gCamera.Zoom) 0.1f 100
	vMat = gCamera.GetViewMatrix();
	//vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, -camZ));

	GLuint multipleTexturesLoc = glGetUniformLocation(shaderProgram, "multipleTextures");
	glUniform1i(multipleTexturesLoc, gIsCrate);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureIdCrate);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTextureIdGrid);







	drawCube2();
	drawCube();
	drawPyramid();
	drawPlane();
	drawCylinder();
	drawBall();
	glUseProgram(0);
}

//**********************************************************************************************
//draw lamp
void draw1(GLFWwindow* window, double currentTime)
{
	//glClear(GL_DEPTH_BUFFER_BIT); // Z-buffer operation (hsr removal)
	//glClear(GL_COLOR_BUFFER_BIT); // remove animation trails

	glUseProgram(lampShaderProgram); // load shaders to GPU

	// Reference matrix uniform variables in shader 
	lampMvLoc = glGetUniformLocation(lampShaderProgram, "mv_matrix");
	lampProjLoc = glGetUniformLocation(lampShaderProgram, "proj_matrix");



	// Build Perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;


	//pMat = glm::perspective(1.0472f, aspectRatio, 0.1f, 1000.0f);
	pMat = glm::perspective(glm::radians(gCamera.Zoom), aspectRatio, 0.1f, 100.0f);  //glm::radians(gCamera.Zoom) 0.1f 100
	vMat = gCamera.GetViewMatrix();
	//vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, -camZ));

	/*
	GLuint multipleTexturesLoc = glGetUniformLocation(shaderProgram, "multipleTextures");
	glUniform1i(multipleTexturesLoc, gIsCrate);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureIdCrate);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTextureIdGrid);
	*/

	drawLight();
}






/*Draw Primitive(s)*/
void drawOrtho(GLFWwindow* window, double currentTime)
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



	/*
	pMat = glm::ortho(-fov, 5.0f, -fov, 5.0f, 0.1f, 100.0f);

	//Build View matrix
	vMat = glm::translate(glm::mat4(3.0f), glm::vec3(-camX, -camY, -camZ)); //1.0f

	mvMat = glm::lookAt(cameraPosition, getTarget(), worldUp);
	*/



	vMat = gCamera.GetViewMatrix();
	pMat = glm::ortho(-glm::radians(gCamera.Zoom), 5.0f, -glm::radians(gCamera.Zoom), 5.0f, 0.1f, 100.0f);
	//vMat = gCamera.GetViewMatrix();

	drawCube();
	drawPyramid();
	drawPlane();

}

void togglePerspective(GLFWwindow* window);


// Set up view (camera), triangle, and plane locations
void initPositions()
{
	camX = 0.0f;
	camY = 0.0f;
	camZ = 10.0f; //10

	planeLocX = 0.0f;
	planeLocY = -1.0f;
	planeLocZ = -13.0f;  //-13

	cubeLocX = 0.0f;
	cubeLocY = 0.0f;
	cubeLocZ = 0.0f;

	pLocX = -0.2f; //-0.2f
	pLocY = 1.5f; //1.5f
	pLocZ = 2.0f; //2.0f
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
	//vertices for lamp
	GLfloat lampVertices[] = {
		// positon attributes (x,y,z)
		 1.0f, 1.0f, 1.0f,


		 -1.0f, 1.0f, 1.0f,


		 -1.0f, -1.0f, 1.0f,


		 -1.0f, -1.0f, 1.0f,


		 1.0f, -1.0f, 1.0f,


		 1.0f, 1.0f, 1.0f,


		 1.0f, 1.0f, 1.0f,


		 1.0f, -1.0f, 1.0f,


		 1.0f, -1.0f, -1.0f,


		 1.0f, -1.0f, -1.0f,


		 1.0f, 1.0f, -1.0f,


		 1.0f, 1.0f, 1.0f,//12


		 1.0f, 1.0f, 1.0f,


		 1.0f, 1.0f, -1.0f,


		 -1.0f, 1.0f, -1.0f,


		 -1.0f, 1.0f, -1.0f,


		 -1.0f, 1.0f, 1.0f,


		 1.0f, 1.0f, 1.0f,


		 -1.0f, 1.0f, 1.0f,


		 -1.0f, 1.0f, -1.0f,


		 -1.0f, -1.0f, -1.0f,



		 -1.0f, -1.0f, -1.0f,


		 -1.0f, -1.0f, 1.0f,


		 -1.0f, 1.0f, 1.0f,
		 //24

		  -1.0f, -1.0f, -1.0f,


		  1.0f, -1.0f, -1.0f,


		  1.0f, -1.0f, 1.0f,


		  1.0f, -1.0f, 1.0f,


		  1.0f, -1.0f, 1.0f,


		  -1.0f, -1.0f, 1.0f,


		  -1.0f, -1.0f, -1.0f,


		  1.0f, -1.0f, -1.0f,


		  -1.0f, -1.0f, -1.0f,


		  -1.0f, 1.0f, -1.0f,


		  1.0f, 1.0f, -1.0f,


		 1.0f, -1.0f, -1.0f,
		 //36


	};






	// Define vertex data for Triangle (GLfloat array)
	GLfloat planeVertices[] = {
		// positon attributes (x,y,z)
		-10.0f, 10.0f, 1.0f,  // vert 1
		1.0f, 0.0f, 0.0f,
		0.0, 0.0,0.0, 0.0, 0.0,    // red

		10.0f, -10.0f, 1.0f, // vert 2
		0.0f, 1.0f, 0.0f,
		0.0, 1.0,1.0, 0.0, 0.0,      // green

		-10.0f, -10.0f, 1.0f, // vert 3
		0.0f, 0.0f, 1.0f,
		1.0, 0.0, 0.0, 0.0, 0.0,    // blue

		-10.0f, 10.0f, 1.0f, // vert 4
		0.0f, 1.0f, 0.0f,
		1.0, 1.0,1.0, -1.0, 0.0,     // green

		10.0f, 10.0f, 1.0f, // vert 5
		0.0f, 1.0f, 0.0f,
		0.0, 0.0,0.0,-1.0, 0.0,     // green

		10.0f, -10.0f, 1.0f, // vert 6
		0.0f, 0.0f, 1.0f,
		0.0, 1.0, 1.0, -1.0, 0.0,          // blue


	};



	GLfloat cubeVertices[] = {
		// positon attributes (x,y,z)
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 0.0, 0.0, 0.0, -1.0,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 0.0, 0.1, 0.0, 0.0, -1.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
		 1.0, 0.0,0.0, 0.0, -1.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
		 1.0, 1.0,0.0, 0.0, -1.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
		 0.0, 0.0,0.0, 0.0, -1.0,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 1.0,0.0, 0.0, -1.0, //6

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0, 0.0, 0.0, 0.0, 1.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
		 1.0, 1.0, 0.0, 0.0, 1.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
		 0.0, 0.0,0.0, 0.0, 1.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
		 0.0, 1.0,0.0, 0.0, 1.0,

		 1.0f, 1.0f, -1.0f,
		 0.0f, 1.1f, 1.1f,
		 1.0, 0.0,0.0, 0.0, 1.0,

		 1.0f, 1.0f, 1.0f,//12
		 1.0f, 1.0f, 1.0f,
		 1.0, 1.0,0.0, 0.0, 1.0,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 0.0, -1.0, 0.0, 0.0,

		 1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 1.0f,
		 0.0, 1.0,-1.0, 0.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0, 0.0,-1.0, 0.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0, 1.0,-1.0, 0.0, 0.0,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 0.0, 0.0,-1.0, 0.0, 0.0,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 1.0,-1.0, 0.0, 0.0, //6

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0, 0.0, 1.0, 0.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0, 1.0,1.0, 0.0, 0.0,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
		 0.0, 0.0,1.0, 0.0, 0.0,


		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
		 0.0, 1.0,1.0, 0.0, 0.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
		 1.0, 0.0,1.0, 0.0, 0.0,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0, 1.0,1.0, 0.0, 0.0,//24

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
			 0.0, 0.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
			 0.0, 1.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
			 1.0, 0.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
			 1.0, 1.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
			 0.0, 0.0, 0.0, -1.0, 0.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
			 0.0, 1.0, 0.0, -1.0, 0.0, //6

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
			 1.0, 0.0, 0.0, 1.0, 0.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
			 1.0, 1.0, 0.0, 1.0, 0.0,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
			 0.0, 0.0, 0.0, 1.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		  0.0f, 1.0f, 0.0f,
			 0.0, 1.0, 0.0, 1.0, 0.0,

		 1.0f, 1.0f, -1.0f,
		0.0f, 1.0f, 1.0f,
			 1.0, 0.0, 0.0, 1.0, 0.0,

		1.0f, -1.0f, -1.0f,
		0.0f, 0.0f, 1.0f,
			 1.0,1.0, 0.0, 1.0, 0.0,//36


	};


	GLfloat cube2Vertices[] = {
		// positon attributes (x,y,z)
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 0.0, 0.0, 0.0, -1.0,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 0.0, 0.1, 0.0, 0.0, -1.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
		 1.0, 0.0,0.0, 0.0, -1.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
		 1.0, 1.0,0.0, 0.0, -1.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
		 0.0, 0.0,0.0, 0.0, -1.0,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 1.0,0.0, 0.0, -1.0, //6

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0, 0.0, 0.0, 0.0, 1.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
		 1.0, 1.0, 0.0, 0.0, 1.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
		 0.0, 0.0,0.0, 0.0, 1.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
		 0.0, 1.0,0.0, 0.0, 1.0,

		 1.0f, 1.0f, -1.0f,
		 0.0f, 1.1f, 1.1f,
		 1.0, 0.0,0.0, 0.0, 1.0,

		 1.0f, 1.0f, 1.0f,//12
		 1.0f, 1.0f, 1.0f,
		 1.0, 1.0,0.0, 0.0, 1.0,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 0.0, -1.0, 0.0, 0.0,

		 1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 1.0f,
		 0.0, 1.0,-1.0, 0.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0, 0.0,-1.0, 0.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0, 1.0,-1.0, 0.0, 0.0,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 0.0, 0.0,-1.0, 0.0, 0.0,

		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 0.0, 1.0,-1.0, 0.0, 0.0, //6

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0, 0.0, 1.0, 0.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0, 1.0,1.0, 0.0, 0.0,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
		 0.0, 0.0,1.0, 0.0, 0.0,


		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
		 0.0, 1.0,1.0, 0.0, 0.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
		 1.0, 0.0,1.0, 0.0, 0.0,

		 -1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0, 1.0,1.0, 0.0, 0.0,//24

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
			 0.0, 0.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
			 0.0, 1.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
			 1.0, 0.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
			 1.0, 1.0, 0.0, -1.0, 0.0,

		 1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 1.0f,
			 0.0, 0.0, 0.0, -1.0, 0.0,

		 -1.0f, -1.0f, 1.0f,
		 1.0f, 0.0f, 0.0f,
			 0.0, 1.0, 0.0, -1.0, 0.0, //6

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
			 1.0, 0.0, 0.0, 1.0, 0.0,

		 1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
			 1.0, 1.0, 0.0, 1.0, 0.0,

		 -1.0f, -1.0f, -1.0f,
		 0.0f, 0.0f, 0.0f,
			 0.0, 0.0, 0.0, 1.0, 0.0,

		 -1.0f, 1.0f, -1.0f,
		  0.0f, 1.0f, 0.0f,
			 0.0, 1.0, 0.0, 1.0, 0.0,

		 1.0f, 1.0f, -1.0f,
		0.0f, 1.0f, 1.0f,
			 1.0, 0.0, 0.0, 1.0, 0.0,

		1.0f, -1.0f, -1.0f,
		0.0f, 0.0f, 1.0f,
			 1.0,1.0, 0.0, 1.0, 0.0,//36


	};







	GLfloat vertices[] = {
		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 1.0, 0.0,

		-1.0, -1.0, 1.0,
		0.0, 1.0, 0.0,
		0.0, 1.0,0.0, 1.0, 0.0,

		1.0, -1.0, 1.0,
		0.0, 0.0, 1.0,
		1.0, 0.0,0.0, 1.0, 0.0,

		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 1.0,0.0, 1.0, 0.0,

		1.0, -1.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0,0.0, 1.0, 0.0,

		1.0, -1.0, -1.0,
		0.0, 1.0, 0.0,
		0.0, 1.0,0.0, 1.0, 0.0,

		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 0.0,0.0, 1.0, 0.0,

		1.0, -1.0, -1.0,
		0.0, 1.0, 0.0,
		1.0,1.0,0.0, 1.0, 0.0,

		-1.0, -1.0, -1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0,0.0, 1.0, 0.0,

		0.0, 1.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 1.0,0.0, 1.0, 0.0,

		-1.0, -1.0, -1.0,
		0.0, 0.0, 1.0,
		1.0, 0.0,0.0, 1.0, 0.0,

		-1.0, -1.0, 1.0,
		0.0, 1.0, 0.0,
		1.0, 1.0,0.0, 1.0, 0.0,

	};


	//000 010 100 110        10
	GLfloat cylinderVertices[] = {
		0.0f, .0f, 0.0f,  // vert 1
		1.0f, 0.0f, 0.0f, 0.0, 0.0, 0.0,1.0, 0.0,  // red

		0.5f, 0.866f, 0.0f, // vert 2
		0.0f, 1.0f, 0.0f, 0.0, 1.0, 0.0, 1.0, 0.0, // green

		1.0f, 0.0f, 0.0f, // vert 3
		0.0f, 0.0f, 1.0f, 1.0, 0.0, 0.0, 1.0, 0.0, // blue

		0.5f, 0.866f, 0.0f, // vert 4
		0.0f, 1.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 0.0, // green

		0.5f, 0.866f, -2.0f, // vert 5
		0.0f, 1.0f, 0.0f, 0.0, 0.0, 0.0, 1.0, 0.0, // green	

		1.0f, 0.0f, 0.0f, // vert 6
		0.0f, 0.0f, 1.0f, 0.0, 1.0, 0.0, 1.0, 0.0, // blue

		1.0f, 0.0f, 0.0f, // vert 7
		0.0f, 0.0f, 1.0f, 1.0, 0.0, 0.0, 1.0, 0.0, // blue

		1.0f, 0.0f, -2.0f, // vert 8
		0.0f, 0.0f, 1.0f, 1.0, 1.0, 0.0, 1.0, 0.0, // blue

		0.5f, 0.866f, -2.0f, // vert 9
		0.0f, 1.0f, 0.0f, 0.0, 0.0, 0.0, 1.0, 0.0 // green
	};

	GLfloat ballVertices[] = {
		0.0f, .0f, 0.0f,  // vert 1
		1.0f, 0.0f, 0.0f, 0.0, 0.0, 0.0,1.0, 0.0,  // red

		0.5f, 0.866f, 0.0f, // vert 2
		0.0f, 1.0f, 0.0f, 0.0, 1.0, 0.0, 1.0, 0.0, // green

		1.0f, 0.0f, 0.0f, // vert 3
		0.0f, 0.0f, 1.0f, 1.0, 0.0, 0.0, 1.0, 0.0, // blue

		0.5f, 0.866f, 0.0f, // vert 4
		0.0f, 1.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 0.0, // green

		0.5f, 0.866f, -2.0f, // vert 5
		0.0f, 1.0f, 0.0f, 0.0, 0.0, 0.0, 1.0, 0.0, // green	

		1.0f, 0.0f, 0.0f, // vert 6
		0.0f, 0.0f, 1.0f, 0.0, 1.0, 0.0, 1.0, 0.0, // blue

		1.0f, 0.0f, 0.0f, // vert 7
		0.0f, 0.0f, 1.0f, 1.0, 0.0, 0.0, 1.0, 0.0, // blue

		1.0f, 0.0f, -2.0f, // vert 8
		0.0f, 0.0f, 1.0f, 1.0, 1.0, 0.0, 1.0, 0.0, // blue

		0.5f, 0.866f, -2.0f, // vert 9
		0.0f, 1.0f, 0.0f, 0.0, 0.0, 0.0, 1.0, 0.0 // green
	};


	//ball
	glGenVertexArrays(1, &ballVAO); // Create VAO
	glGenBuffers(1, &ballVBO); // Create VBO
	glBindVertexArray(ballVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(ballVertices), ballVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(2); // Enable VA

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(3); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)



	//cylinder
	glGenVertexArrays(1, &cylinderVAO); // Create VAO
	glGenBuffers(1, &cylinderVBO); // Create VBO
	glBindVertexArray(cylinderVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderVertices), cylinderVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(2); // Enable VA

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(3); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)




	glGenVertexArrays(1, &planeVAO); // Create VAO
	glGenBuffers(1, &planeVBO); // Create VBO
	glBindVertexArray(planeVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO); // Enable VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(2); // Enable VA

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(3); // Enable VA


	glBindVertexArray(0); // Unbind VAO (Optional but recommended)


	glGenVertexArrays(1, &cubeVAO); // Create VAO
	glGenBuffers(1, &cubeVBO); // Create VBO
	glBindVertexArray(cubeVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(2); // Enable VA

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(3); // Enable VA

	glBindVertexArray(0); // Unbind VAO (Optional but recommended)



	//CUBE2
	glGenVertexArrays(1, &cube2VAO); // Create VAO
	glGenBuffers(1, &cube2VBO); // Create VBO
	glBindVertexArray(cube2VAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, cube2VBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube2Vertices), cube2Vertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(2); // Enable VA

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(3); // Enable VA

	glBindVertexArray(0); // Unbind VAO (Optional but recommended)



	//vertices
	glGenVertexArrays(1, &verticesVAO); // Create VAO
	glGenBuffers(1, &verticesVBO); // Create VBO
	glBindVertexArray(verticesVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(1); // Enable VA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(2); // Enable VA
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); // Associate VBO color with VA
	glEnableVertexAttribArray(3); // Enable VA

	glBindVertexArray(0); // Unbind VAO (Optional but recommended)

	//FOR LAMP
	glGenVertexArrays(1, &lampVAO); // Create VAO
	glGenBuffers(1, &lampVBO); // Create VBO
	glBindVertexArray(lampVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO position with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)

}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, 0, STBI_rgb);
	if (image)
	{
		flipImageVertically(image, width, height, channels = 0);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 0)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// else if (channels == 4)
			// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}


void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
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

	// Set input callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1); // VSync operation


	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	initPositions();// Set up view (camera) and triangle initial positions
	CreateVertices(); // Generate vertices, create VAO, create buffers, associate VAO with VBO, load vertices to VBOs, associate VBOs with VAs (Vertex Attributes)


	// Load textures
	const char* texFilename = "crate.png";
	if (!UCreateTexture(texFilename, gTextureIdCrate))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	texFilename = "brick.jpg";
	if (!UCreateTexture(texFilename, gTextureIdGrid))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(shaderProgram);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(shaderProgram, "uTextureBase"), 0);
	// We set the texture as texture unit 1
	glUniform1i(glGetUniformLocation(shaderProgram, "uTextureExtra"), 1);

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);



	// Write vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec4 aColor;"
		"layout(location = 2) in vec2 textureCoordinate;"
		"layout(location = 3) in vec3 normal;"


		"out vec4 v_color;"
		"out vec2 vertexTextureCoordinate;"
		"out vec3 oNormal;"
		"out vec3 FragPos;"

		"uniform mat4 mv_matrix;"
		"uniform mat4 proj_matrix;"
		"void main()\n"
		"{\n"
		"gl_Position = proj_matrix * mv_matrix * vec4(vPosition.x,vPosition.y,vPosition.z, 1.0 );"
		" v_color = aColor;"
		"vertexTextureCoordinate = textureCoordinate;"
		"oNormal = mat3(transpose(inverse(mv_matrix)))* normal;"
		"FragPos = vec3(mv_matrix * vec4(vPosition, 1.0f));"
		"}\n";

	// Write fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec4 v_color;"
		"in vec2 vertexTextureCoordinate;"
		"in vec3 oNormal;"
		"in vec3 FragPos;"


		"out vec4 fragColor;"
		"uniform mat4 mv_matrix;"
		"uniform mat4 proj_matrix;"

		"uniform sampler2D uTextureBase;"
		"uniform sampler2D uTextureExtra;"
		"uniform bool multipleTextures;"
		"uniform vec3 objectColor;"
		"uniform vec3 lightColor;"
		"uniform vec3 lightPos;"
		"uniform vec3 viewPos;"
		"void main()\n"
		"{\n"
		"float ambientStrength = 0.1f;"
		"vec3 ambient = ambientStrength * lightColor;"
		"vec3 norm = normalize(oNormal);"
		"vec3 lightDir = normalize(lightPos - FragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"vec3 diffuse = diff * lightColor;"
		"float specularStrength = 1.5f;"
		"vec3 viewDir = normalize(viewPos - FragPos);"
		"vec3 reflectDir = reflect(-lightDir, norm);"
		"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);"
		"vec3 specular = specularStrength * spec * lightColor;"
		"vec3 result = (ambient+diffuse+specular) * objectColor;"
		//"fragColor = v_color;"
		"fragColor = texture(uTextureBase, vertexTextureCoordinate) * vec4(result, 1.0f);"
		"}\n";


	//********************************************* lamp shader

	// lamp vertex shader source code
	string lampVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 mv_matrix;"
		"uniform mat4 proj_matrix;"
		"void main()\n"
		"{\n"
		"gl_Position = proj_matrix * mv_matrix * vec4(vPosition.x,vPosition.y,vPosition.z, 1.0 );"
		"}\n";

	// Lamp fragment shader source code
	string lampfragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		//where I can change color of the lamp
		"fragColor = vec4(1.0f);"
		"}\n";




	// Create shader (program object)
	shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampfragmentShaderSource);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{


		//set delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);


		//*****************************************************************************************************DRAW
		// Draw primitives
		draw(window, glfwGetTime());
		//glUseProgram(0);
		togglePerspective(window);
		draw1(window, glfwGetTime());


		/*
		if (!isOrtho) {
			draw(window, glfwGetTime());


		}
		if (isOrtho) {
			drawOrtho(window, glfwGetTime());


		}
		*/
		UDestroyTexture(gTextureIdCrate);
		UDestroyTexture(gTextureIdGrid);

		/* Swap front and back buffers */
		glfwSwapBuffers(window); // VSync operation

		/* Poll for and process events */
		glfwPollEvents(); // Detect keyboard and mouse input
	}

	glfwTerminate();
	return 0;
}








//Define Input Callback functions
void key_callback(GLFWwindow* windwo, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

	// Clamp FOV
	if (fov >= 1.f && fov <= 45.f)
		fov -= yoffset * 0.01f;

	// Default FOV
	if (fov < 1.f)
		fov = 1.f;
	if (fov > 45.f)
		fov = 45.f;

}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	//calculate cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	//pan camera
	if (isPanning)
	{
		if (cameraPosition.z < 0.f)
			cameraFront.z = 1.f;
		else
			cameraFront.z = -1.f;

		GLfloat cameraSpeed = xChange * deltaTime;
		cameraPosition += cameraSpeed * cameraRight;

		cameraSpeed = yChange * deltaTime;
		cameraPosition += cameraSpeed * cameraUp;
	}

	//Orbit camera
	if (isOrbiting)
	{
		rawYaw += xChange;
		rawPitch += yChange;

		//Convert Yaw and Pitch to degrees
		degYaw = glm::radians(rawYaw);
		//degPitch = glm::radians(rawPitch);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.f + .1f, glm::pi<float>() / 2.f - .1f);

		// Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);
	}

}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}

// Define getTarget function
glm::vec3 getTarget() {

	if (isPanning)
		target = cameraPosition + cameraFront;

	return target;
}

//Define TransformCamera function
void TransformCamera() {

	//pan Camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE])
		isPanning = true;
	else
		isPanning = false;

	//Orbit camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE])
		isOrbiting = true;
	else
		isOrbiting = false;

	//Reset camera
	if (keys[GLFW_KEY_F])
		initCamera();

}

void initCamera() {
	cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPosition - target);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));


}

void togglePerspective(GLFWwindow* window) {




	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		draw(window, glfwGetTime());

	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		drawOrtho(window, glfwGetTime());
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, deltaTime);

}






