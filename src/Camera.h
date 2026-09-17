#pragma once
#include <glm/glm.hpp>

// Simple spherical orbit camera: drag to rotate around the origin, scroll
// to zoom.
class Camera {
public:
    explicit Camera(double distance = 25.0);

    glm::mat4 GetViewMatrix() const;
    glm::vec3 GetPosition() const;

    void Rotate(double dx, double dy);
    void Zoom(double dy);

private:
    double yaw = -90.0;
    double pitch = 15.0;
    double distance;
    double minDistance = 2.0;
    double maxDistance = 600.0;
    glm::vec3 target{0.0f, 0.0f, 0.0f};
};
