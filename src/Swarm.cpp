#include <iostream>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "core.h"

int main(int argc, char** argv) {
	App app =  App();
	app.app_loop();
	glfwTerminate();
	return 0;
}

