#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <Camera_Helper.hpp>

class Camera
{
public: 

	void init();
	void reinitPos();
	void update(float _deltaTime, GLFWwindow* _window);
	void updateInterface(float _deltaTime);
	void updateFreeInput(float _deltaTime, GLFWwindow* _window);
	void updateFontRightUp();
	void computeFinalView();
	void mouseRotation(float _deltaTime, GLFWwindow* window);
	void keybordRotation(float _deltaTime, GLFWwindow* window);

	glm::quat getRotation() const {return m_rotation;}
	glm::mat4 getViewMatrix() const {return m_viewMatrix;}
	glm::mat4 getProjectionMatrix() const {return m_projectionMatrix;}

private:

	//Camera parameters 
	float		m_fovDegree{ 45.0f };
	float		m_speedTranslation{ 10.0f };
	float		m_speedRotation{ 2.0f };
	glm::vec3	m_position{ glm::vec3(0.f, 40.f, 0.f) };
	glm::vec3	m_eulerAngle{ glm::vec3(0.f, 0.f, 0.f) };
	glm::vec3	m_front = VEC_FRONT;
	glm::vec3	m_right = VEC_RIGHT;
	glm::vec3	m_up = VEC_UP;
	glm::quat	m_rotation{};
	float lastX = 0;
	float lastY = 0;

	int		m_mode{ 0 };
	int		m_mode_axe_Horizontal{ -1 };
	int		m_mode_axe_Vertical{ -1 };
	bool	m_mode_free_cursor=false;

	//Interface option
	bool m_showImguiDemo{ false };

	//View
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;
};