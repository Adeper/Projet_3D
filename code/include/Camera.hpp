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
	float		m_speedRotation{ 0.01f };
	glm::vec3	m_position{ glm::vec3(0.f, 40.f, 0.f) };
	glm::vec3	m_eulerAngle{ glm::vec3(0.f, 0.f, 0.f) };
	glm::vec3	m_front = VEC_FRONT;
	glm::vec3	m_right = VEC_RIGHT;
	glm::vec3	m_up = VEC_UP;
	glm::vec3	m_front_horizontal = VEC_FRONT;
	glm::vec3	m_right_horizontal = VEC_RIGHT;
	glm::quat	m_rotation{};
	float		m_far = 200.0f;

	int		m_mode_axe_Horizontal{ 1 };
	int		m_mode_axe_Vertical{ 1 };
	int		m_mode_camera{ 0 };
	int		m_nb_mode = 2;

	//Pour la rotation de la camera avec le click souris
	bool isRotating = false;
	glm::vec2 lastPos{ glm::vec2(0.,0.) };

	//Interface option
	bool m_showImguiDemo{ false };

	//View
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;
};