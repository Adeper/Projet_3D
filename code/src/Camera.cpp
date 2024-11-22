#include <Camera.hpp>
#include <Camera_Helper.hpp>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

glm::vec3	position_initial{ glm::vec3(0.f, 5.f, 0.f) };
glm::vec3	euler_initial{ glm::vec3(0.f, 0.f, 0.f) };

void Camera::init()
{
	m_fovDegree = 45.0f;
	m_speedTranslation = 10.0f;
	m_speedRotation = 0.1f;
	m_position = position_initial;
	m_eulerAngle = euler_initial;
	m_front = VEC_FRONT;
	m_right = VEC_RIGHT;
	m_front_horizontal = VEC_FRONT;
	m_right_horizontal = VEC_RIGHT;
	m_up = VEC_UP;
	m_rotation = glm::quat{};
	m_showImguiDemo = false;
	m_far = 500.0f;
}

bool isChecked_X = false;
bool isChecked_Y = false;
void Camera::updateInterface(float _deltaTime)
{

	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	// ImGUI window creation
	if (ImGui::Begin("Info camera"))
	{
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
		ImGui::SliderFloat("FOV", &m_fovDegree, 1.0f, 150.0f);

		ImGui::Text("Far clip %f", m_far);
		ImGui::SliderFloat("Far clip", &m_far, 50.0f, 1000.0f);

		ImGui::Separator();
		ImGui::Text(" === Les controles et modes ===");
		if (ImGui::Checkbox("Inverser l'axe X", &isChecked_X)) {
			if (isChecked_X) {
				std::cout << "L'axe x est inverse" << std::endl;
				m_mode_axe_Horizontal = -1;
			} else {
				std::cout << "L'axe x n'est pas inverse" << std::endl;
				m_mode_axe_Horizontal = 1;
			}
		}
		if (ImGui::Checkbox("Inverser l'axe Y", &isChecked_Y)) {
			if (isChecked_Y) {
				std::cout << "L'axe y est inverse" << std::endl;
				m_mode_axe_Vertical = -1;
			} else {
				std::cout << "L'axe y n'est pas inverse" << std::endl;
				m_mode_axe_Vertical = 1;
			}
		}

		ImGui::Text("Mode camera : %d", m_mode_camera);
		if (ImGui::Button("Changer de mode de mouvement")){
			m_mode_camera = (m_mode_camera+1) % m_nb_mode;
		}

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

void Camera::updateFreeInput(float _deltaTime, GLFWwindow* _window)
{
	if (!ImGui::GetIO().WantCaptureMouse) {
		if ((glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && !isRotating){
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			isRotating = true;
			double xpos, ypos;
			glfwGetCursorPos(_window, &xpos, &ypos);
			lastPos = glm::vec2(xpos, ypos);
		}
		
		if ((glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) && isRotating){
			isRotating = false;
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}else{
		isRotating = false;
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	switch(m_mode_camera) {
	case 0: // Mode 0 : mouvement par rapport à l'orientation de la camera
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
		break;
	case 1: // Mode 1 : mouvement uniquement par rapport au front et right (orientation pas comprise)
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
		break;
	default:
		std::cerr << "Mode camera incorrect !" << std::endl;
	}

	// Move up (world's up vector)
	if (glfwGetKey( _window, GLFW_KEY_SPACE ) == GLFW_PRESS){
		m_position += VEC_UP * _deltaTime * m_speedTranslation;
	}
	// Move down (world's up vector)
	if (glfwGetKey( _window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
		m_position -= VEC_UP * _deltaTime * m_speedTranslation;
	}
}

void Camera::updateFontRightUp(){
	m_eulerAngle.y = Camera_Helper::clipAngle180(m_eulerAngle.y);

	if(m_eulerAngle.x > 90.)
		m_eulerAngle.x = 89.999;
	if(m_eulerAngle.x < -90.)
		m_eulerAngle.x = -89.999;
	
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
	updateFreeInput(_deltaTime, _window);

	if (isRotating) {
		mouseRotation(_deltaTime, _window);
	}

	updateFontRightUp();
	m_rotation = glm::quat(glm::radians(m_eulerAngle));

	Camera_Helper::computeFinalView(m_projectionMatrix, m_viewMatrix, m_position, m_rotation, m_fovDegree, m_far);
}

void Camera::mouseRotation(float _deltaTime, GLFWwindow* window) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    glm::vec2 currentMousePos(xpos, ypos);

    glm::vec2 delta = currentMousePos - lastPos;

    lastPos = currentMousePos;

    m_eulerAngle.x += delta.y * m_speedRotation * m_mode_axe_Vertical; //pitch
    m_eulerAngle.y -= delta.x * m_speedRotation * m_mode_axe_Horizontal; //yaw
}
