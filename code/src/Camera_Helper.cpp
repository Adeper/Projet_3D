#include <Camera_Helper.hpp>



glm::vec3 Camera_Helper::quatToEuler(glm::quat _quat)
{
	// Opengl quat to euler function give yaw betweel -90 and 90
	// If you want correct pitch and yaw you can use this 
	// Src http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
	// But roll will be bewteen -90 and 90 here

	glm::vec3 eulerAngles;

	float test = _quat.x * _quat.y + _quat.z * _quat.w;

	if (test > 0.499f) { // singularity at north pole
		eulerAngles.y = 2.f * atan2(_quat.x, _quat.w);
		eulerAngles.z = M_PI / 2.f;
		eulerAngles.x = 0.f;
		return eulerAngles;
	}
	if (test < -0.499f) { // singularity at south pole
		eulerAngles.y = -2.f * atan2(_quat.x, _quat.w);
		eulerAngles.z = -M_PI / 2.f;
		eulerAngles.x = 0.f;
		return eulerAngles;
	}
	float sqx = _quat.x * _quat.x;
	float sqy = _quat.y * _quat.y;
	float sqz = _quat.z * _quat.z;


	eulerAngles.y = atan2(2.f * _quat.y * _quat.w - 2.f * _quat.x * _quat.z, 1.f - 2.f * sqy - 2.f * sqz);
	eulerAngles.z = asin(2.f * test);
	eulerAngles.x = atan2(2.f * _quat.x * _quat.w - 2.f * _quat.y * _quat.z, 1.f - 2.f * sqx - 2.f * sqz);

	return eulerAngles;
}



void Camera_Helper::computeFinalView(glm::mat4& _outProjectionMatrix, glm::mat4& _outviewMatrix, glm::vec3& _position, glm::quat _rotation, float _fovDegree, float far)
{
	// Projection matrix : FOV, 4:3 ratio, display range : 0.1 unit <-> 100 units
	_outProjectionMatrix = glm::perspective(glm::radians(_fovDegree), 4.0f / 3.0f, 0.1f, far);

	const glm::vec3 front = normalize(_rotation* VEC_FRONT);
	const glm::vec3 up = normalize(_rotation * VEC_UP);

	// Camera matrix
	_outviewMatrix = glm::lookAt(_position, _position + front, up);
}

glm::vec3 Camera_Helper::ProjectVectorOnPlan(const glm::vec3& vector, const glm::vec3& normal_plan) {
    glm::vec3 normal_plan_normalized = glm::normalize(normal_plan);

    float dot_product = glm::dot(vector, normal_plan_normalized);

    glm::vec3 parallel_component = dot_product * normal_plan_normalized;

    glm::vec3 vecteur_projection = vector - parallel_component;

    return vecteur_projection;
}

float Camera_Helper::clipAngle180(float angle) {
    angle = fmod(angle, 360.0f);
    angle = angle >= 180.0f ? angle - 360.0f : (angle < -180.0f ? angle + 360.0f : angle);

    return angle;
}

float Camera_Helper::interpolationCosinus(float ratio) {
    return 0.5f - 0.5f * std::cos(ratio * M_PI);
}
