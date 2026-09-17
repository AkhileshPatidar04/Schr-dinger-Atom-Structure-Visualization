#include "Wavefunction.h"
#include <cmath>
#include <algorithm>

namespace wave {

double factorial(int n)
{
    if (n <= 1) return 1.0;
    double result = 1.0;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

double assocLaguerre(int k, int alpha, double x)
{
    // L_0^alpha = 1, L_1^alpha = 1 + alpha - x, then upward recurrence:
    // k * L_k^alpha = (2k - 1 + alpha - x) * L_{k-1}^alpha
    //               - (k - 1 + alpha) * L_{k-2}^alpha
    if (k < 0) return 0.0;
    if (k == 0) return 1.0;

    double Lm2 = 1.0;                    // L_0
    double Lm1 = 1.0 + alpha - x;        // L_1
    if (k == 1) return Lm1;

    double Lk = Lm1;
    for (int i = 2; i <= k; ++i) {
        Lk = ((2 * i - 1 + alpha - x) * Lm1 - (i - 1 + alpha) * Lm2) / i;
        Lm2 = Lm1;
        Lm1 = Lk;
    }
    return Lk;
}

double assocLegendre(int l, int m, double x)
{
    // Standard stable recurrence, m >= 0, |x| <= 1.
    m = std::abs(m);

    // P_m^m
    double pmm = 1.0;
    if (m > 0) {
        double somx2 = std::sqrt(std::max(0.0, (1.0 - x) * (1.0 + x)));
        double fact = 1.0;
        for (int i = 1; i <= m; ++i) {
            pmm *= -fact * somx2;
            fact += 2.0;
        }
    }
    if (l == m) return pmm;

    // P_{m+1}^m
    double pmmp1 = x * (2.0 * m + 1.0) * pmm;
    if (l == m + 1) return pmmp1;

    // Upward recurrence to P_l^m
    double pll = 0.0;
    for (int ll = m + 2; ll <= l; ++ll) {
        pll = ((2.0 * ll - 1.0) * x * pmmp1 - (ll + m - 1.0) * pmm) / (ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
    }
    return pll;
}

double radialWaveFunction(int n, int l, double r, double Z)
{
    double rho = 2.0 * Z * r / n;

    double norm = std::sqrt(
        std::pow(2.0 * Z / n, 3) * factorial(n - l - 1) /
        (2.0 * n * factorial(n + l))
    );

    return norm * std::exp(-rho / 2.0) * std::pow(rho, l) *
           assocLaguerre(n - l - 1, 2 * l + 1, rho);
}

double realSphericalHarmonic(int l, int m, double theta, double phi)
{
    double x = std::cos(theta);
    int am = std::abs(m);

    double plm = assocLegendre(l, am, x);
    double norm = std::sqrt(
        (2.0 * l + 1.0) / (4.0 * M_PI) *
        factorial(l - am) / factorial(l + am)
    );

    if (m == 0) return norm * plm;
    if (m > 0)  return std::sqrt(2.0) * norm * plm * std::cos(m * phi);
    return std::sqrt(2.0) * norm * plm * std::sin(am * phi);
}

double psi(int n, int l, int m, double r, double theta, double phi, double Z)
{
    return radialWaveFunction(n, l, r, Z) * realSphericalHarmonic(l, m, theta, phi);
}

WaveEvaluator::WaveEvaluator(int n, int l, int m, double Z)
    : n(n), l(l), m(m), Z(Z)
{
    am = std::abs(m);
    rhoFactor = 2.0 * Z / n;
    radNorm = std::sqrt(
        std::pow(rhoFactor, 3) * factorial(n - l - 1) /
        (2.0 * n * factorial(n + l))
    );
    ylmNorm = std::sqrt(
        (2.0 * l + 1.0) / (4.0 * M_PI) *
        factorial(l - am) / factorial(l + am)
    );
    if (m != 0) {
        ylmNorm *= std::sqrt(2.0);
    }
    kLaguerre = n - l - 1;
    alphaLaguerre = 2 * l + 1;
}

double WaveEvaluator::evalDensity(double x, double y, double z, double& signedPsi) const
{
    double r2 = x * x + y * y + z * z;
    if (r2 < 1e-12) { signedPsi = 0.0; return 0.0; }
    double r = std::sqrt(r2);

    // Radial part
    double rho = rhoFactor * r;
    double R = radNorm * std::exp(-rho * 0.5) * std::pow(rho, l) *
               assocLaguerre(kLaguerre, alphaLaguerre, rho);

    // Angular part: cos(theta) is exactly z / r! No acos() needed.
    double cosTheta = std::clamp(z / r, -1.0, 1.0);
    double plm = assocLegendre(l, am, cosTheta);

    double Y;
    if (m == 0) {
        Y = ylmNorm * plm;
    } else {
        double phi = std::atan2(y, x);
        if (m > 0) {
            Y = ylmNorm * plm * std::cos(m * phi);
        } else {
            Y = ylmNorm * plm * std::sin(am * phi);
        }
    }

    signedPsi = R * Y;
    return signedPsi * signedPsi;
}

} // namespace wave
