#include <Camera.hpp>
#include <Camera_Helper.hpp>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

glm::vec3	position_initial{ glm::vec3(0.f, 40.f, 0.f) };
glm::vec3	euler_initial{ glm::vec3(0.f, 0.f, 0.f) };

float dureeTransition = 5.;
float tempsEcoule = 0.;


void Camera::init()
{
	m_fovDegree = 45.0f;
	m_speedTranslation = 10.0f;
	m_speedRotation = 2.0f;
	m_position = position_initial;
	m_eulerAngle = euler_initial;
	m_front = VEC_FRONT;
	m_right = VEC_RIGHT;
	m_front_horizontal = VEC_FRONT;
	m_right_horizontal = VEC_RIGHT;
	m_up = VEC_UP;
	m_rotation = glm::quat{};
	m_mode = 0;
	m_showImguiDemo = false;
}

void Camera::transition(float _deltaTime){
	tempsEcoule += _deltaTime;
	float ratio = Camera_Helper::interpolationCosinus(tempsEcoule / (dureeTransition * dureeTransition));

	m_position = glm::mix(m_position, position_initial, ratio);

	m_rotation = glm::slerp(m_rotation, glm::quat(glm::radians(euler_initial)), ratio);

	m_eulerAngle = glm::degrees(glm::eulerAngles(m_rotation));

	if (tempsEcoule >= dureeTransition) {
		m_inTransition = false;
		tempsEcoule = 0.;
	}
}

void Camera::updateInterface(float _deltaTime)
{
	// ImGUI window creation
	if (ImGui::Begin("Interface"))
	{
		ImGui::Separator();
		ImGui::Text("Welcome to this TP about Cameras! Press escape to close the exe");
		ImGui::Text("Long live to the cameras");
		ImGui::Separator();
		ImGui::Text("Position camera (%f %f %f)", m_position.x, m_position.y, m_position.z);
		ImGui::SliderFloat("x", &m_position.x, -50.0f, 50.0f);
		ImGui::SliderFloat("y", &m_position.y, -50.0f, 50.0f);
		ImGui::SliderFloat("z", &m_position.z, -50.0f, 50.0f);
		ImGui::SliderFloat("vitesse translation", &m_speedTranslation, 0.0f, 20.0f);
		ImGui::SliderFloat("vitesse rotation", &m_speedRotation, 0.0f, 5.0f);

		ImGui::Text("Angle camera (%f %f %f)", m_eulerAngle.x, m_eulerAngle.y, m_eulerAngle.z);
		ImGui::SliderFloat("pitch", &m_eulerAngle.x, -180, 180);
		ImGui::SliderFloat("yaw", &m_eulerAngle.y, -180, 180);
		ImGui::SliderFloat("roll", &m_eulerAngle.z, -180, 180);

		ImGui::Text("FOV %f", m_fovDegree);
		ImGui::SliderFloat("FOV", &m_fovDegree, 1.0f, 179.0f);

		ImGui::Text("Duree transition %f", dureeTransition);
		ImGui::SliderFloat("Duree transition", &dureeTransition, 1.0f, 20.0f);

		ImGui::Text("tempsEcoule (%f)", tempsEcoule);

		ImGui::Separator();
		ImGui::Text(" === Les controles et modes ===");
		ImGui::Text("Faire apparaitre/disparaitre le curseur : press left_Ctrl");
		ImGui::Text("Changer Mode : %d (press left_Shift)", m_mode);
		ImGui::Text("Inversion axe X : %d (press 1)", m_mode_axe_Horizontal);
		ImGui::Text("Inversion axe Y : %d (press 2)", m_mode_axe_Vertical);

		ImGui::Separator();
		if (ImGui::Button("Réinitialisation (press r)"))
    		m_inTransition = true;
		ImGui::Separator();
		if (ImGui::Button("Réinitialisation (Tout)"))
    		init();
	}
	ImGui::End();

	if (m_showImguiDemo)
	{
		ImGui::ShowDemoWindow();
	}

}

//for toggles (to prevent when the key stays down)
bool isLeftShiftRealised = false;
bool isLeftControlRealised = false;
bool isOneRealised = false;
bool isTwoRealised = false;
bool isRRealised = false;
void Camera::updateFreeInput(float _deltaTime, GLFWwindow* _window)
{
	//Toggle mode
	if (glfwGetKey( _window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS && !isLeftShiftRealised){
		m_mode = (m_mode + 1)%2;
		isLeftShiftRealised = true;
	}
	if(glfwGetKey( _window, GLFW_KEY_LEFT_SHIFT ) == GLFW_RELEASE){
		isLeftShiftRealised = false;
	}

	//transition
	if (glfwGetKey( _window, GLFW_KEY_R ) == GLFW_PRESS && !isRRealised){
		m_inTransition = true;
		isRRealised = true;
	}
	if(glfwGetKey( _window, GLFW_KEY_R ) == GLFW_RELEASE){
		isRRealised = false;
	}

	// Switch between visible or hidden cursor
	if (glfwGetKey( _window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS && !isLeftControlRealised){
		if(!m_mode_free_cursor)
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
		else
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

		m_mode_free_cursor = !m_mode_free_cursor;
		isLeftControlRealised = true;
	}
	if(glfwGetKey( _window, GLFW_KEY_LEFT_CONTROL ) == GLFW_RELEASE){
		isLeftControlRealised = false;
	}

	//Inverse X axe
	if ((glfwGetKey( _window, GLFW_KEY_1) == GLFW_PRESS || (glfwGetKey( _window, GLFW_KEY_KP_1) == GLFW_PRESS)) && !isOneRealised){
		m_mode_axe_Horizontal *= -1;
		isOneRealised = true;
	}
	if(glfwGetKey( _window, GLFW_KEY_1 ) == GLFW_RELEASE && glfwGetKey( _window, GLFW_KEY_KP_1 ) == GLFW_RELEASE){
		isOneRealised = false;
	}

	//Inverse Y axe
	if ((glfwGetKey( _window, GLFW_KEY_2) == GLFW_PRESS || (glfwGetKey( _window, GLFW_KEY_KP_2) == GLFW_PRESS)) && !isTwoRealised){
		m_mode_axe_Vertical *= -1;
		isTwoRealised = true;
	}
	if(glfwGetKey( _window, GLFW_KEY_2 ) == GLFW_RELEASE && glfwGetKey( _window, GLFW_KEY_KP_2 ) == GLFW_RELEASE){
		isTwoRealised = false;
	}

	if(!m_mode_free_cursor){
		if(m_mode == 0){
			mouseRotation(_deltaTime, _window);

			// Move forward
			if (glfwGetKey( _window, GLFW_KEY_W ) == GLFW_PRESS){
				m_position += m_front * _deltaTime * m_speedTranslation;
			}
			// Move backward
			if (glfwGetKey( _window, GLFW_KEY_S ) == GLFW_PRESS){
				m_position -= m_front * _deltaTime * m_speedTranslation;
			}
			// Strafe right
			if (glfwGetKey( _window, GLFW_KEY_D ) == GLFW_PRESS){
				m_position += m_right * _deltaTime * m_speedTranslation;
			}
			// Strafe left
			if (glfwGetKey( _window, GLFW_KEY_A ) == GLFW_PRESS){
				m_position -= m_right * _deltaTime * m_speedTranslation;
			}

		}else if(m_mode == 1){
			// Move forward
			if (glfwGetKey( _window, GLFW_KEY_W ) == GLFW_PRESS){
				m_position += m_front_horizontal * _deltaTime * m_speedTranslation;
			}
			// Move backward
			if (glfwGetKey( _window, GLFW_KEY_S ) == GLFW_PRESS){
				m_position -= m_front_horizontal * _deltaTime * m_speedTranslation;
			}
			// Strafe right
			if (glfwGetKey( _window, GLFW_KEY_D ) == GLFW_PRESS){
				m_position += m_right_horizontal * _deltaTime * m_speedTranslation;
			}
			// Strafe left
			if (glfwGetKey( _window, GLFW_KEY_A ) == GLFW_PRESS){
				m_position -= m_right_horizontal * _deltaTime * m_speedTranslation;
			}

			// pitch up
			if (glfwGetKey( _window, GLFW_KEY_UP ) == GLFW_PRESS){
				m_eulerAngle.x += 5. * m_speedRotation * _deltaTime * m_mode_axe_Vertical;
			}
			// pitch down
			if (glfwGetKey( _window, GLFW_KEY_DOWN ) == GLFW_PRESS){
				m_eulerAngle.x -= 5. * m_speedRotation * _deltaTime * m_mode_axe_Vertical;
			}
			// yaw right
			if (glfwGetKey( _window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
				m_eulerAngle.y += 5. * m_speedRotation * _deltaTime * m_mode_axe_Horizontal;
			}
			// yaw left
			if (glfwGetKey( _window, GLFW_KEY_LEFT ) == GLFW_PRESS){
				m_eulerAngle.y -= 5. * m_speedRotation * _deltaTime * m_mode_axe_Horizontal;
			}
		}
	}
}

void Camera::updateFontRightUp(){
	m_eulerAngle.y = Camera_Helper::clipAngle180(m_eulerAngle.y);

	if(m_eulerAngle.x > 90.)
		m_eulerAngle.x = 90.;
	if(m_eulerAngle.x < -90.)
		m_eulerAngle.x = -90.;
	
    m_front.x = glm::sin(glm::radians(m_eulerAngle.y)) * glm::cos(glm::radians(m_eulerAngle.x));
    m_front.y = -glm::sin(glm::radians(m_eulerAngle.x));
    m_front.z = glm::cos(glm::radians(m_eulerAngle.y)) * glm::cos(glm::radians(m_eulerAngle.x));

	m_front = glm::normalize(m_front);
	
	m_right = glm::normalize(glm::cross(m_front, VEC_UP));

	m_up = glm::normalize(glm::cross(m_right, m_front));

	m_right = glm::normalize(glm::cross(m_front, m_up));

	// front et right pour camera 2
	m_front_horizontal.x = m_front.x;
	m_front_horizontal.y = 0.;
	m_front_horizontal.z = m_front.z;

	m_front_horizontal = glm::normalize(m_front_horizontal);

	m_right_horizontal = glm::normalize(glm::cross(m_front_horizontal, VEC_UP));
	m_right_horizontal = glm::normalize(glm::cross(m_front_horizontal, glm::normalize(glm::cross(m_right_horizontal, m_front_horizontal))));
}

void Camera::update(float _deltaTime, GLFWwindow* _window)
{
	updateInterface(_deltaTime);
	if(!m_inTransition)
		updateFreeInput(_deltaTime, _window);
	else
		transition(_deltaTime);

	updateFontRightUp();
	m_rotation = glm::quat(glm::radians(m_eulerAngle));

	Camera_Helper::computeFinalView(m_projectionMatrix, m_viewMatrix, m_position, m_rotation, m_fovDegree);
}

void Camera::mouseRotation(float _deltaTime, GLFWwindow* window){
	double pos_cursor_horizontal, pos_cursor_vertical;
	glfwGetCursorPos(window, &pos_cursor_horizontal, &pos_cursor_vertical);

	double offset_horizontal = pos_cursor_horizontal - lastX;
	double offset_vertical = lastY - pos_cursor_vertical;
	lastX = pos_cursor_horizontal;
	lastY = pos_cursor_vertical;

	offset_horizontal *= m_speedRotation * _deltaTime;
	offset_vertical *= m_speedRotation * _deltaTime;

	m_eulerAngle.x += offset_vertical * m_mode_axe_Vertical; //pitch
	m_eulerAngle.y += offset_horizontal * m_mode_axe_Horizontal; //yaw
}