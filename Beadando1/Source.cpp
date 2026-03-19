#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

using namespace std;

GLuint renderingProgram;
GLFWwindow *window = nullptr;
GLuint	vao_line, vbo_line;
GLuint	vao, vbo;


//Setup the beginning value for the circle
float circleCenterX = 300.0f;
float circleCenterY = 300.0f;
float circleVelocityX = 3.0f;
float circleVelocityY = 0.0f;
const float radius = 50.0f;

//Set the line Y to 0.0 to easily be modified later
float lineCenterY = 0.0f;

string readShaderSource(const char* filePath) { //Reads the shaders
	ifstream file(filePath);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

GLuint createShaderProgram() { //Creates the shaders as a vfProgram thatbeen use to connect it to the source.cpp
	string vertShaderStr = readShaderSource("vertexShader.glsl");
	string fragShaderStr = readShaderSource("fragmentShader.glsl"); //Reading the shaders

	const char* vertShaderSrc = vertShaderStr.c_str(); //Creates a char array from them
	const char* fragShaderSrc = fragShaderStr.c_str();

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	glCompileShader(fShader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader); //Attaches the shaders to the Program
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram); //Links them

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return vfProgram;
}

void display(GLFWwindow* window) {

	glClear(GL_COLOR_BUFFER_BIT); //Clears the color buffer
	glUseProgram(renderingProgram); //Tells the program that we wanna use shaders

	//Setup the length of the line
	float lineStartX = -0.3333f;
	float lineEndX = 0.3333f;

	//Makes sure that the circle is in the center of the screen
	float nx = (circleCenterX - 300.0f) / 300.0f;
	float ny = (circleCenterY - 300.0f) / 300.0f;

	//Creates the next position to the circle
	float nextX = circleCenterX + circleVelocityX;

	//Checks if the cirle hits the wall, makes the circcle bounce of from the wall
	if (nextX + radius > 600.0f) {
		float overlap = (nextX + radius) - 600.0f;
		circleVelocityX *= -1;
		nextX = 600.0f - radius - overlap;
	}
	else if (nextX - radius < 0.0f) {
		float overlap = 0.0f - (nextX - radius);
		circleVelocityX *= -1;
		nextX = radius + overlap;
	}

	//Swaps the centerX to the new position X
	circleCenterX = nextX;

	//Save the current data of the line
	GLfloat lineVertices[] = {
		-0.3333f , lineCenterY,
		 0.3333f ,lineCenterY
	};

	//Creates a buffer array, than tells what is the data inside
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);

	//Checks if the Circle and the Line intersects each other 
	bool intersect =
		(nx >= lineStartX && nx <= lineEndX) &&
		(abs(ny - lineCenterY) <= radius / 300.0f);

	//Asks if the bool is true or false, if true it calls the shader's color swap method 
	glUniform1i(glGetUniformLocation(renderingProgram, "colorSwap"), intersect ? 0 : 1);
	glUniform1i(glGetUniformLocation(renderingProgram, "isLine"), 0);
	glUniform2f(glGetUniformLocation(renderingProgram, "circleCenter"), circleCenterX, circleCenterY);
	glUniform1f(glGetUniformLocation(renderingProgram, "radius"), radius);

	//Draws the circle
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 100);

	//Draws the Line 
	glUniform1i(glGetUniformLocation(renderingProgram, "isLine"), 1);
	glBindVertexArray(vao_line);
	glLineWidth(3.0f);
	glDrawArrays(GL_LINES, 0, 2);
	
	glfwSwapBuffers(window);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) { //Makes the program close if escape is pressed
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_DOWN) { //Makes the line go down 
		lineCenterY -= 0.01f;
	}

	if (key == GLFW_KEY_UP) { //Makes the line go up
		lineCenterY += 0.01f;
	}
}

void inicialize() {//Create the shaders and the required data arrays
	renderingProgram = createShaderProgram();

	const int numVertices = 100;
	GLfloat vertices[2 * numVertices];

	for (int i = 0; i < numVertices; i++) {
		float angle = 2.0f * M_PI * i / numVertices;
		vertices[2 * i] = cos(angle);
		vertices[2 * i + 1] = sin(angle);
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Base array for circle drawing

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint positionAttrib = glGetAttribLocation(renderingProgram, "position");
	glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(positionAttrib);

	GLfloat lineVertices[] = {
		-0.3333f, 0.0f,
		 0.3333f, 0.0f
	};

	glGenBuffers(1, &vbo_line);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &vao_line);
	glBindVertexArray(vao_line);
	glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(positionAttrib); //Base array for line drawing

}

int main() {
	if (!glfwInit()) return -1; //Checks if the base of the program is ok

	GLFWwindow* window = glfwCreateWindow(600, 600, "Szamitogepes graf beadando 1", NULL, NULL); //
	glfwMakeContextCurrent(window);
	glewInit();

	glClearColor(217.0f / 255.0f, 211.0f / 255.0f, 30.0f / 255.0f,1.0); //Yellow background

	glfwSetKeyCallback(window, key_callback);

	inicialize(); //Inicialize

	while (!glfwWindowShouldClose(window)) {
		display(window);
		glfwPollEvents(); //Processing Events
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}