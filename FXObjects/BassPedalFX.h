#pragma once

#include "LBFX.h"
#include "../BassPedalFunctions.h"

struct FatPunchParameters {
	FatPunchParameters() {}

	FatPunchParameters& operator=(const FatPunchParameters& params) {
		if (this == &params)
			return *this;

		inDistAmt = params.inDistAmt;
		punchCompOn = params.punchCompOn;
		fatOn = params.fatOn;
		darkenOn = params.darkenOn;

		return *this;
	}

	float inDistAmt = 1.0;
	bool punchCompOn = true;
	bool fatOn = true;
	bool darkenOn = true;
};
struct MelodyModeParameters {
	MelodyModeParameters() {}

	MelodyModeParameters& operator=(const MelodyModeParameters& params) {
		if (this == &params) return *this;
		on = params.on;
		return *this;
	}

	bool on = false;
};
/*

Object for "fat/punchy" sound for low bass rhythm

By: Lucas Burkholder

*/

class FatPunch {
public:
	FatPunch() {}
	~FatPunch() {}

 	virtual bool reset(double _sampleRate) {
		lpeq.reset(_sampleRate);
		hsf.reset(_sampleRate);
		compressor.reset(_sampleRate);
		return true;
	}

	virtual double processAudioSample(double xn) {
		// Step 1 - Distortion
		if (parameters.fatOn)
			xn = tanhWaveShaper(xn, parameters.inDistAmt); //distAmt goes 0.01-10
		// Step 2 - EQ
		if (parameters.fatOn) {
			xn *= dB2Raw(-6.0);
			xn = lpeq.processAudioSample(xn);
		
		}
		if (parameters.darkenOn)
			xn = hsf.processAudioSample(xn);

		// Step 3 - Compression
		if (parameters.punchCompOn)
			xn = compressor.processAudioSample(xn);

		return xn;
		//left to right: fat, dark, punch, melody
	}

	virtual bool canProcessAudioFrame() {
		return false;
	}

	FatPunchParameters getParameters() {
		return parameters;
	}

	void setParameters(const FatPunchParameters& _parameters) {
		if (parameters.inDistAmt != _parameters.inDistAmt 
			|| parameters.punchCompOn != _parameters.punchCompOn
			|| parameters.fatOn != _parameters.fatOn
			|| parameters.darkenOn != _parameters.darkenOn ) {
			parameters = _parameters;
		}
		else return;

		//clamp any parameter values here (like Q >= 0)
		if (parameters.inDistAmt == 0) parameters.inDistAmt = 0.01;

		//update sub-object parameters here
		LB_PEQParameters lpeqParams = lpeq.getParameters();
		lpeqParams.fc = 100; //was 75
		lpeqParams.gain = 8.0;
		lpeqParams.Q = 0.4;
		lpeq.setParameters(lpeqParams);

		LB_HSFParameters hsfParams = hsf.getParameters();
		hsfParams.fc = 700.0;
		hsfParams.gain = -14.0;
		hsf.setParameters(hsfParams);

		LB_CompressorParameters compressorParams = compressor.getParameters();
		compressorParams.attackTime = 150.0;
		compressorParams.releaseTime = 20.0;
		compressorParams.ratio = 3.0;
		compressorParams.threshold_dB = -42.0; //39.1 //assumes peaking around -5 -10dB
		compressorParams.outputGain = 3.0;
		compressor.setParameters(compressorParams);

		//update filter coefficients here? 
		//don't have to bc [filter].setParameters() does it
	}

protected:
	FatPunchParameters parameters;

	LB_PEQ lpeq;
	LB_HSF hsf;

	LB_Compressor compressor;
};

class MelodyMode {
public:
	MelodyMode() {}
	~MelodyMode() {}
	virtual bool reset(double sampleRate) {
		//reset all member fx objects here
		midEQ.reset(sampleRate);
		hiEQ.reset(sampleRate);
		return true;
	}

	virtual double processAudioSample(double xn) {
		//processing here
		//this ASSUMES input signal is within ideal range of -40dB to -25dB
		//	when testing, set input gain to -4dB to get this range

		if (!parameters.on) return xn;

		// Distortion/Saturation
		double yn = waveShaper(xn);

		// EQ mid
		yn = midEQ.processAudioSample(yn);

		// Attenuate to make up for distortion boosts
		yn *= 0.3;

		// EQ high
		yn = hiEQ.processAudioSample(yn);

		
		// Output
		return yn;
	}

	virtual bool canProcessAudioFrame() { return false; }

	MelodyModeParameters getParameters() {
		return parameters;
	}

	void setParameters(const MelodyModeParameters& _parameters) {
		if (parameters.on != _parameters.on) {
			parameters = _parameters;
		}
		else return;

		//clamp any parameters here

		//update sub-object parameters here
		LB_PEQParameters midEQParams = midEQ.getParameters();
		midEQParams.fc = 1118.0;
		midEQParams.gain = 10.0;
		midEQParams.Q = 0.3;
		midEQ.setParameters(midEQParams);

		LB_PEQParameters hiEQParams = hiEQ.getParameters();
		hiEQParams.fc = 4763.0;
		hiEQParams.gain = 5.9;
		hiEQParams.Q = 0.6;
		hiEQ.setParameters(hiEQParams);
	}

private:
	MelodyModeParameters parameters;
	LB_PEQ midEQ, hiEQ;

	double waveShaper(double x) {
		float k = 5.4;
		double num = tanh(k * x * 1.33) * 0.35;
		double den = tanh(k);
		return num / den;
	}

};
