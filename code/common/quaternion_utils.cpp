#include <common/quaternion_utils.hpp>


// Returns a glm::quaternion such that q*start = dest
glm::quat QuaternionUtils::RotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
	start = normalize(start);
	dest = normalize(dest);
	
	float cosTheta = dot(start, dest);
	glm::vec3 rotationAxis;
	
	if (cosTheta < -1 + 0.001f){
		// special case when vectors in opposite directions :
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		// This implementation favors a rotation around the Up axis,
		// since it's often what you want to do.
		rotationAxis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
			rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
		
		rotationAxis = normalize(rotationAxis);
		return angleAxis(glm::radians(180.0f), rotationAxis);
	}

	// Implementation from Stan Melax's Game Programming Gems 1 article
	rotationAxis = cross(start, dest);

	float s = sqrt( (1+cosTheta)*2 );
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f, 
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);


}



// Returns a glm::quaternion that will make your object looking towards 'direction'.
// Similar to RotationBetweenVectors, but also controls the vertical orientation.
// This assumes that at rest, the object faces +Z.
// Beware, the first parameter is a direction, not the target point !
glm::quat QuaternionUtils::LookAt(glm::vec3 direction, glm::vec3 desiredUp){

	if (length2(direction) < 0.0001f )
		return glm::quat();

	// Recompute desiredUp so that it's perpendicular to the direction
	// You can skip that part if you really want to force desiredUp
	glm::vec3 right = cross(direction, desiredUp);
	desiredUp = cross(right, direction);
	
	// Find the rotation between the front of the object (that we assume towards +Z,
	// but this depends on your model) and the desired direction
	glm::quat rot1 = RotationBetweenVectors(glm::vec3(0.0f, 0.0f, 1.0f), direction);
	// Because of the 1rst rotation, the up is probably completely screwed up. 
	// Find the rotation between the "up" of the rotated object, and the desired up
	glm::vec3 newUp = rot1 * glm::vec3(0.0f, 1.0f, 0.0f);
	glm::quat rot2 = RotationBetweenVectors(newUp, desiredUp);
	
	// Apply them
	return rot2 * rot1; // remember, in reverse order.
}



// Like SLERP, but forbids rotation greater than maxAngle (in radians)
// In conjunction to LookAt, can make your characters 
glm::quat QuaternionUtils::RotateTowards(glm::quat q1, glm::quat q2, float maxAngle){
	
	if( maxAngle < 0.001f ){
		// No rotation allowed. Prevent dividing by 0 later.
		return q1;
	}
	
	float cosTheta = dot(q1, q2);
	
	// q1 and q2 are already equal.
	// Force q2 just to be sure
	if(cosTheta > 0.9999f){
		return q2;
	}
	
	// Avoid taking the long path around the sphere
	if (cosTheta < 0){
		q1 = q1*-1.0f;
		cosTheta *= -1.0f;
	}
	
	float angle = acos(cosTheta);
	
	// If there is only a 2� difference, and we are allowed 5�,
	// then we arrived.
	if (angle < maxAngle){
		return q2;
	}

	// This is just like slerp(), but with a custom t
	float t = maxAngle / angle;
	angle = maxAngle;
	
	glm::quat res = (glm::sin((1.0f - t) * angle) * q1 + glm::sin(t * angle) * q2) / glm::sin(angle);
	res = normalize(res);
	return res;
	
}




















void tests(){

	glm::vec3 Xpos(+1.0f,  0.0f,  0.0f);
	glm::vec3 Ypos( 0.0f, +1.0f,  0.0f);
	glm::vec3 Zpos( 0.0f,  0.0f, +1.0f);
	glm::vec3 Xneg(-1.0f,  0.0f,  0.0f);
	glm::vec3 Yneg( 0.0f, -1.0f,  0.0f);
	glm::vec3 Zneg( 0.0f,  0.0f, -1.0f);
	
	// Testing standard, easy case
	// Must be 90� rotation on X : 0.7 0 0 0.7
	glm::quat X90rot = QuaternionUtils::RotationBetweenVectors(Ypos, Zpos);
	
	// Testing with v1 = v2
	// Must be identity : 0 0 0 1
	glm::quat id = QuaternionUtils::RotationBetweenVectors(Xpos, Xpos);
	
	// Testing with v1 = -v2
	// Must be 180� on +/-Y axis : 0 +/-1 0 0
	glm::quat Y180rot = QuaternionUtils::RotationBetweenVectors(Xpos, Xneg);
	
	// Testing with v1 = -v2, but with a "bad first guess"
	// Must be 180� on +/-Y axis : 0 +/-1 0 0
	glm::quat X180rot = QuaternionUtils::RotationBetweenVectors(Zpos, Zneg);
	

}