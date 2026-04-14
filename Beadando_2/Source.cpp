#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Globális változók az ablakkezeléshez
int windowWidth = 800;
int windowHeight = 600;
GLFWwindow* window;

// OpenGL objektumok azonosítói
GLuint program;
GLuint VAO, VBO;
GLint locMatProj, locMatModelView, locUColor;

// Adatok tárolása fix méretû tömbben (vector helyett)
glm::vec2 controlPoints[100];
int pointCount = 0;
int draggedPointIdx = -1;

// Shader betöltõ függvény
GLuint LoadShaders(const char* vsPath, const char* fsPath) {
    std::string vsCode;
    std::ifstream vsFile(vsPath);
    if (vsFile.is_open()) {
        std::stringstream buffer;
        buffer << vsFile.rdbuf();
        vsCode = buffer.str();
    }

    std::string fsCode;
    std::ifstream fsFile(fsPath);
    if (fsFile.is_open()) {
        std::stringstream buffer;
        buffer << fsFile.rdbuf();
        fsCode = buffer.str();
    }

    const char* vsSource = vsCode.c_str();
    const char* fsSource = fsCode.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, NULL);
    glCompileShader(fs);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}

// Rekurzív de Casteljau algoritmus a görbe pontjainak kiszámításához
glm::vec2 getBezierPoint(float t, glm::vec2* pts, int n) {
    if (n == 1) return pts[0];
    glm::vec2 temporaryPts[100];
    for (int i = 0; i < n - 1; i++) {
        temporaryPts[i] = glm::mix(pts[i], pts[i + 1], t);
    }
    return getBezierPoint(t, temporaryPts, n - 1);
}

void display() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    // Vetítési mátrixok beállítása
    float aspect = (float)windowWidth / (float)windowHeight;
    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
    glm::mat4 mv = glm::mat4(1.0f);

    glUniformMatrix4fv(locMatProj, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(locMatModelView, 1, GL_FALSE, glm::value_ptr(mv));

    if (pointCount == 0) return;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Control Poligon
    glUniform3f(locUColor, 1.0f, 1.0f, 1.0f);
    glBufferData(GL_ARRAY_BUFFER, pointCount * sizeof(glm::vec2), controlPoints, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);

    // Bazier
    if (pointCount >= 2) {
        glm::vec2 curvePoints[101];
        for (int i = 0; i <= 100; i++) {
            curvePoints[i] = getBezierPoint(i / 100.0f, controlPoints, pointCount);
        }
        glUniform3f(locUColor, (float) 196/255, (float) 230/255, (float) 30/255);
        glBufferData(GL_ARRAY_BUFFER, 101 * sizeof(glm::vec2), curvePoints, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_STRIP, 0, 101);
    }

    // control point
    glPointSize(9.0f);
    glUniform3f(locUColor, (float) 41/255, (float) 63/255, (float) 171/255);
    glBufferData(GL_ARRAY_BUFFER, pointCount * sizeof(glm::vec2), controlPoints, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_POINTS, 0, pointCount);
}

void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    float aspect = (float)windowWidth / (float)windowHeight;
    glm::vec2 m((x / windowWidth * 2.0f - 1.0f) * aspect, 1.0f - y / windowHeight * 2.0f);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Meglévõ pont keresése kijelöléshez
            for (int i = 0; i < pointCount; i++) {
                if (glm::distance(m, controlPoints[i]) < 0.05f) {
                    draggedPointIdx = i;
                    return;
                }
            }
            // Új pont hozzáadása
            if (pointCount < 100) {
                controlPoints[pointCount++] = m;
            }
        }
        else if (action == GLFW_RELEASE) {
            draggedPointIdx = -1;
        }
    }

    // Pont törlése jobb klikkel
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        for (int i = 0; i < pointCount; i++) {
            if (glm::distance(m, controlPoints[i]) < 0.05f) {
                for (int j = i; j < pointCount - 1; j++) {
                    controlPoints[j] = controlPoints[j + 1];
                }
                pointCount--;
                break;
            }
        }
    }
}

void cursorPosCallback(GLFWwindow* w, double x, double y) {
    if (draggedPointIdx != -1) {
        float aspect = (float)windowWidth / (float)windowHeight;
        controlPoints[draggedPointIdx] = glm::vec2((x / windowWidth * 2.0f - 1.0f) * aspect, 1.0f - y / windowHeight * 2.0f);
    }
}

int main() {
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(800, 600, "Beadando 2", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    program = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Uniform helyek lekérése
    locMatProj = glGetUniformLocation(program, "matProjection");
    locMatModelView = glGetUniformLocation(program, "matModelView");
    locUColor = glGetUniformLocation(program, "uColor");

    // Bufferek inicializálása
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // Kerek pontok beállítása
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);

    // Kezdõ 4 pont beállítása
    controlPoints[0] = glm::vec2(-0.6f, -0.4f);
    controlPoints[1] = glm::vec2(-0.2f, 0.5f);
    controlPoints[2] = glm::vec2(0.2f, 0.5f);
    controlPoints[3] = glm::vec2(0.6f, -0.4f);
    pointCount = 4;

    // Callbackek regisztrálása
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        display();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

    return 0;
}