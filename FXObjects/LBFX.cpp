#include "LBFX.h"

double LBBiquad::processAudioSample(double xn) {
	// Canonical form difference eqn
	double wn = xn - coeffArray[b1] * stateArray[x_z1]
		- coeffArray[b2] * stateArray[x_z2];
	
	double yn = coeffArray[a0] * wn
		+ coeffArray[a1] * stateArray[x_z1]
		+ coeffArray[a2] * stateArray[x_z2];

	//update state registers
	stateArray[x_z2] = stateArray[x_z1];
	stateArray[x_z1] = wn;

	return yn;
}

double LB_LPF::processAudioSample(double xn) {
	return biquad.processAudioSample(xn);
}

bool LB_LPF::calculateFilterCoeffs() {
	//clear coeff array
	memset(&coeffArray[0], 0, sizeof(double) * numCoeffs);

	// --- set default pass-through
	coeffArray[a0] = 1.0;
	coeffArray[c0] = 1.0;
	coeffArray[d0] = 0.0;


	//for 2nd order low pass filter
	double thetaC = 2 * kPi * parameters.fc / sampleRate;
	double d = 1.0 / parameters.Q;
	double beta = 0.5 * (1 - (d / 2) * sin(thetaC)) / (1 + (d / 2) * sin(thetaC));
	double gamma = (0.5 + beta) * cos(thetaC);
	
	coeffArray[a0] = (0.5 + beta - gamma) / 2.0;
	coeffArray[a1] = 0.5 + beta - gamma;
	coeffArray[a2] = coeffArray[a0];
	coeffArray[b1] = -2 * gamma;
	coeffArray[b2] = 2 * beta;

	biquad.setCoefficients(coeffArray);

	return true;

}

double LB_PEQ::processAudioSample(double xn) {
	return coeffArray[d0] * xn + coeffArray[c0] * biquad.processAudioSample(xn);
}

bool LB_PEQ::calculateFilterCoeffs() {
	// non-constant Q parametric EQ

	memset(&coeffArray, 0, sizeof(double) * numCoeffs);

	double fc = parameters.fc;
	double Q = parameters.Q;
	double gain = parameters.gain;
	double thetaC = 2.0 * kPi * fc / sampleRate; //try changing sample rate
	double mu = pow(10.0, gain / 20.0);
	double zeta = 4.0 / (1.0 + mu);

	double tanInput = thetaC / (2.0 * Q);
	if (tanInput > 0.95 * kPi / 2.0) tanInput = 0.95 * kPi / 2.0; //clamp to 0.95 * pi/2, since tan(pi/2) is undefined

	double betaNum = 1.0 - (zeta * tan(tanInput));
	double betaDen = 1.0 + (zeta * tan(tanInput));
	double beta = 0.5 * betaNum / betaDen;
	double gamma = (0.5 + beta) * cos(thetaC);

	coeffArray[a0] = 0.5 - beta;
	coeffArray[a1] = 0.0;
	coeffArray[a2] = -1.0 * (0.5 - beta);
	coeffArray[b1] = -2.0 * gamma;
	coeffArray[b2] = 2.0 * beta;
	coeffArray[c0] = mu - 1.0;
	coeffArray[d0] = 1.0;

	

	biquad.setCoefficients(coeffArray);

	return true;

}

bool LB_HSF::calculateFilterCoeffs() {
	//reset coefficients
	memset(&coeffArray, 0, sizeof(double) * numCoeffs);

	double fc = parameters.fc;
	double gain = parameters.gain;

	double thetaC = 2.0 * kPi * fc / sampleRate;
	double mu = pow(10.0, gain / 20.0);
	double beta = (1.0 + mu) / 4.0;
	double tanInput = thetaC / 2;

	// clamp tan input
	if (tanInput == kPi / 2.0) tanInput = 0.95 * kPi / 2.0; 

	double delta = beta * tan(tanInput);
	double gamma = (1.0 - delta) / (1.0 + delta);

	coeffArray[a0] = (1.0 + gamma) / 2.0;
	coeffArray[a1] = (1.0 + gamma) / -2.0;
	coeffArray[a2] = 0.0;
	coeffArray[b1] = -1.0 * gamma;
	coeffArray[b2] = 0.0;
	coeffArray[c0] = mu - 1.0;
	coeffArray[d0] = 1.0;

	biquad.setCoefficients(coeffArray);

	return true;

}

double LB_HSF::processAudioSample(double xn) {
	return coeffArray[d0] * xn + coeffArray[c0] * biquad.processAudioSample(xn);
	
}