#pragma once
#include <vector>
#include <cstddef>
#include <glm/glm.hpp>

struct OrbitalPoint {
    glm::vec3 pos;
    float sign; // sign of psi at this point (+1 / -1), used for coloring
};

// Generates a point cloud whose density approximates |psi_nlm|^2 via
// rejection sampling, and renders it as GL_POINTS.
class OrbitalCloud {
public:
    ~OrbitalCloud();

    void Generate(int n, int l, int m, double Z, int numPoints);
    void Draw() const;

    double BoundingRadius() const { return boundingRadius; }

private:
    unsigned int VAO = 0, VBO = 0;
    size_t pointCount = 0;
    double boundingRadius = 10.0;

    void Upload(const std::vector<OrbitalPoint>& points);
};
