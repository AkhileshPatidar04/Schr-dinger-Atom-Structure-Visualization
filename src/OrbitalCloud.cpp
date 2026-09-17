#include "OrbitalCloud.h"
#include "Wavefunction.h"
#include <GL/glew.h>
#include <random>
#include <cmath>
#include <algorithm>
#include <thread>
#include <future>
#include <vector>

void OrbitalCloud::Generate(int n, int l, int m, double Z, int numPoints)
{
    // Bounding cube half-extent, tightly containing > 99.8% of the electron
    // probability density without wasting cycles on empty outer space.
    boundingRadius = (2.5 * n * n + 8.0) / Z;

    wave::WaveEvaluator eval(n, l, m, Z);

    // Estimate peak density by probing with a 20% safety margin.
    std::mt19937 probeRng(std::random_device{}());
    std::uniform_real_distribution<double> boxDist(-boundingRadius, boundingRadius);
    double densityMax = 0.0;
    const int probeCount = 20000;
    for (int i = 0; i < probeCount; ++i) {
        double dummy;
        densityMax = std::max(densityMax, eval.evalDensity(boxDist(probeRng), boxDist(probeRng), boxDist(probeRng), dummy));
    }
    densityMax = std::max(densityMax * 1.20, 1e-300);

    // Multi-threaded Monte Carlo rejection sampling
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    int pointsPerThread = numPoints / numThreads;

    std::vector<std::future<std::vector<OrbitalPoint>>> futures;
    futures.reserve(numThreads);

    for (unsigned int t = 0; t < numThreads; ++t) {
        int target = (t == numThreads - 1) ? (numPoints - pointsPerThread * (numThreads - 1)) : pointsPerThread;
        futures.push_back(std::async(std::launch::async, [&eval, this, densityMax, target, t]() {
            std::random_device rd;
            std::mt19937 rng(rd() + t * 997);
            std::uniform_real_distribution<double> dist(-boundingRadius, boundingRadius);
            std::uniform_real_distribution<double> unit(0.0, 1.0);

            std::vector<OrbitalPoint> localPoints;
            localPoints.reserve(target);

            long long attempts = 0;
            const long long maxAttempts = static_cast<long long>(target) * 20000;

            while (static_cast<int>(localPoints.size()) < target && attempts < maxAttempts) {
                ++attempts;
                double x = dist(rng), y = dist(rng), z = dist(rng);
                double signedPsi;
                double density = eval.evalDensity(x, y, z, signedPsi);

                if (unit(rng) * densityMax < density) {
                    OrbitalPoint p;
                    p.pos = glm::vec3((float)x, (float)y, (float)z);
                    p.sign = signedPsi >= 0.0 ? 1.0f : -1.0f;
                    localPoints.push_back(p);
                }
            }
            return localPoints;
        }));
    }

    std::vector<OrbitalPoint> points;
    points.reserve(numPoints);
    for (auto& f : futures) {
        auto threadPoints = f.get();
        points.insert(points.end(), threadPoints.begin(), threadPoints.end());
    }

    Upload(points);
}

void OrbitalCloud::Upload(const std::vector<OrbitalPoint>& points)
{
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
    pointCount = points.size();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(OrbitalPoint),
                 points.empty() ? nullptr : points.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OrbitalPoint),
                           (void*)offsetof(OrbitalPoint, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(OrbitalPoint),
                           (void*)offsetof(OrbitalPoint, sign));

    glBindVertexArray(0);
}

void OrbitalCloud::Draw() const
{
    if (VAO == 0 || pointCount == 0) return;
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(pointCount));
    glBindVertexArray(0);
}

OrbitalCloud::~OrbitalCloud()
{
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}
