#pragma once

// Hydrogen-like (one-electron) time-independent Schrödinger equation
// solutions, in atomic units with the Bohr radius a0 = 1.
//
//   psi_nlm(r, theta, phi) = R_nl(r) * Y_lm(theta, phi)
//
// R_nl uses the associated Laguerre polynomials, Y_lm is the REAL
// (chemistry-convention) spherical harmonic built from the associated
// Legendre polynomials, so psi is a real, signed function whose square
// gives the electron probability density |psi|^2.
namespace wave {

// n! as a double (fine for the small n used here).
double factorial(int n);

// Associated Laguerre polynomial L_k^alpha(x).
double assocLaguerre(int k, int alpha, double x);

// Associated Legendre polynomial P_l^m(x), m >= 0.
double assocLegendre(int l, int m, double x);

// Radial part of the hydrogen-like wavefunction, R_nl(r), for nuclear
// charge Z (Z = 1 for hydrogen, Z = 2 for He+, ...).
double radialWaveFunction(int n, int l, double r, double Z);

// Real spherical harmonic Y_lm(theta, phi), -l <= m <= l.
double realSphericalHarmonic(int l, int m, double theta, double phi);

// Full wavefunction value (signed, real) at a point in spherical
// coordinates.
double psi(int n, int l, int m, double r, double theta, double phi, double Z);

// Fast wavefunction evaluator for repeated sampling with precomputed normalization constants.
struct WaveEvaluator {
    int n, l, m;
    double Z;
    int am;
    double radNorm;
    double ylmNorm;
    double rhoFactor;
    int kLaguerre;
    int alphaLaguerre;

    WaveEvaluator(int n, int l, int m, double Z);

    // Evaluates density and signed wavefunction directly from Cartesian coordinates (x, y, z).
    // Computes cos(theta) = z / r directly to eliminate expensive acos() calls.
    double evalDensity(double x, double y, double z, double& signedPsi) const;
};

} // namespace wave
