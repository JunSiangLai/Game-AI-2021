
#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"

//Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

class Application
{
public:
	static Application& GetInstance()
	{
		static Application app;
		return app;
	}

	void Init();
	void Run();
	void Iterate();
	void Exit();
	static bool IsKeyPressed(unsigned short key);
	static bool IsMousePressed(unsigned short key);
	static void GetCursorPos(double *xpos, double *ypos);
	static int GetWindowWidth();
	static int GetWindowHeight();
	// Mouse Wheel
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static bool m_ismousescrollymoved;
	static float m_mousescrollx;
	static float m_mousescrolly;

private:
	Application();
	~Application();

	//Declare a window object
	StopWatch m_timer;
};

#endif