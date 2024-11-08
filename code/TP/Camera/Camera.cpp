#include <TP/Camera/Camera.hpp>
#include <TP/Camera/Camera_Helper.hpp>

// Include GLM
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


void Camera::init()
{
	m_fovDegree = 45.0f;
	m_speedTranslation = 10.0f;
	m_speedRotation = 2.0f;
	m_position = glm::vec3(0.f, 40.f, 0.f);
	m_eulerAngle = glm::vec3(0.f, 0.f, 0.f);
	m_front = VEC_FRONT;
	m_right = VEC_RIGHT;
	m_up = VEC_UP;
	m_rotation = glm::quat{};
	m_mode = 0;
	m_showImguiDemo = false;
}

void Camera::reinitPos(){
	m_position = glm::vec3(0.f, 40.f, 0.f);
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
		ImGui::SliderFloat("pitch", &m_eulerAngle.x, -M_PI, M_PI);
		ImGui::SliderFloat("yaw", &m_eulerAngle.y, -M_PI, M_PI);
		ImGui::SliderFloat("roll", &m_eulerAngle.z, -M_PI, M_PI);

		ImGui::Text("FOV %f", m_fovDegree);
		ImGui::SliderFloat("FOV", &m_fovDegree, 1.0f, 179.0f);

		ImGui::Text("Front (%f %f %f)", m_front.x, m_front.y, m_front.z);
		ImGui::Text("Front worl (%f %f %f)", VEC_FRONT.x, VEC_FRONT.y, VEC_FRONT.z);

		ImGui::Separator();
		ImGui::Text(" === Les controles et modes ===");
		ImGui::Text("Faire apparaitre/disparaitre le curseur : press left_Ctrl");
		ImGui::Text("Mode : %d (press tab)", m_mode);
		ImGui::Text("Inversion axe X : %d (press 1)", m_mode_axe_Horizontal);
		ImGui::Text("Inversion axe Y : %d (press 2)", m_mode_axe_Vertical);

		ImGui::Separator();
		if (ImGui::Button("Réinitialisation (Position)"))
    		reinitPos();
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
bool isTabRealised = false;
bool isLeftControlRealised = false;
bool isOneRealised = false;
bool isTwoRealised = false;
void Camera::updateFreeInput(float _deltaTime, GLFWwindow* _window)
{
	//Toggle mode
	if (glfwGetKey( _window, GLFW_KEY_TAB ) == GLFW_PRESS && !isTabRealised){
		m_mode = (m_mode + 1)%2;
		isTabRealised = true;
	}
	if(glfwGetKey( _window, GLFW_KEY_TAB ) == GLFW_RELEASE){
		isTabRealised = false;
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

	if(!m_mode_free_cursor){
		if(m_mode == 0){
			mouseRotation(_deltaTime, _window);

		}/*else if(m_mode == 1){
			keybordRotation(_deltaTime, _window);

			// Move up
			if (glfwGetKey( _window, GLFW_KEY_E ) == GLFW_PRESS){
				m_position += up * _deltaTime * m_speedTranslation;
			}
			// Move down
			if (glfwGetKey( _window, GLFW_KEY_Q ) == GLFW_PRESS){
				m_position -= up * _deltaTime * m_speedTranslation;
			}

			// pitch up
			if (glfwGetKey( _window, GLFW_KEY_UP ) == GLFW_PRESS){
				pos_cursor_vertical += 10;
			}
			// pitch down
			if (glfwGetKey( _window, GLFW_KEY_DOWN ) == GLFW_PRESS){
				pos_cursor_vertical -= 10;
			}

			// yaw right
			if (glfwGetKey( _window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
				pos_cursor_horizontal += 10;
			}
			// yaw left
			if (glfwGetKey( _window, GLFW_KEY_LEFT ) == GLFW_PRESS){
				pos_cursor_horizontal -= 10;
			}
		}*/
	}
}

void Camera::updateFontRightUp(){
	
    m_front.x = glm::sin(m_eulerAngle.y) * glm::cos(m_eulerAngle.x);
    m_front.y = -glm::sin(m_eulerAngle.x);
    m_front.z = glm::cos(m_eulerAngle.y) * glm::cos(m_eulerAngle.x);

	m_front = glm::normalize(m_front);
	
	m_right = glm::normalize(glm::cross(m_front, VEC_UP));

	m_up = glm::normalize(glm::cross(m_right, m_front));

	m_right = glm::normalize(glm::cross(m_front, m_up));
}

void Camera::update(float _deltaTime, GLFWwindow* _window)
{
	updateInterface(_deltaTime);
	updateFreeInput(_deltaTime, _window);

	m_rotation = glm::quat(m_eulerAngle);

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

	m_eulerAngle.x += glm::radians(offset_vertical * m_mode_axe_Vertical); //pitch
	m_eulerAngle.y += glm::radians(offset_horizontal * m_mode_axe_Horizontal); //yaw

	updateFontRightUp();
}

/*void Camera::keybordRotation(float _deltaTime, GLFWwindow* window){
	double offset_horizontal = pos_cursor_horizontal - lastX;
	double offset_vertical = lastY - pos_cursor_vertical;
	lastX = pos_cursor_horizontal;
	lastY = pos_cursor_vertical;

	offset_horizontal *= m_speedRotation * _deltaTime;
	offset_vertical *= m_speedRotation * _deltaTime;

	if(offset_vertical > 89.0f)
        offset_vertical = 89.0f;
    if(offset_vertical < -89.0f)
        offset_vertical = -89.0f;

	m_eulerAngle.x += glm::radians(offset_vertical * m_mode_axe_Vertical); //pitch
	m_eulerAngle.y += glm::radians(offset_horizontal * m_mode_axe_Horizontal); //yaw
}*/