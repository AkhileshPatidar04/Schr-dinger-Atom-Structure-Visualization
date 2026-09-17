#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera(double dist) : distance(dist) {}

glm::vec3 Camera::GetPosition() const
{
    double yawRad = glm::radians(yaw);
    double pitchRad = glm::radians(pitch);

    float x = static_cast<float>(distance * std::cos(pitchRad) * std::cos(yawRad));
    float y = static_cast<float>(distance * std::sin(pitchRad));
    float z = static_cast<float>(distance * std::cos(pitchRad) * std::sin(yawRad));

    return target + glm::vec3(x, y, z);
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(GetPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::Rotate(double dx, double dy)
{
    yaw += dx * 0.3;
    pitch += dy * 0.3;
    pitch = std::clamp(pitch, -89.0, 89.0);
}

void Camera::Zoom(double dy)
{
    distance -= dy * (distance * 0.1 + 0.5);
    distance = std::clamp(distance, minDistance, maxDistance);
}
