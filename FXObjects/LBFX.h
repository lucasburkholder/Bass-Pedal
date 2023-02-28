#include <cstring>
#include <math.h>
#pragma once

enum filterCoeff { a0, a1, a2, b1, b2, c0, d0, numCoeffs };
enum stateReg { x_z1, x_z2, y_z1, y_z2, numStates };

//Constants
const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
const double kSmallestNegativeFloatValue = -1.175494351e-38;         /* min negative value */
const double TLD_AUDIO_ENVELOPE_ANALOG_TC = -0.99967234081320612357829304641019; // ln(36.7%)
const double kPi = 3.14159265358979323846;

/* 
Biquad object, by Lucas Burkholder
*/

class LBBiquad {

public:

	LBBiquad() {} //Constructor
	~LBBiquad() {} //Destructor

	bool reset(double _sampleRate) {
		memset(&stateArray[0], 0, sizeof(double) * numStates);
		return true;
	}

	virtual double processAudioSample(double xn);

	bool canProcessAudioFrame() { return false; }

	void setCoefficients(double* coeffs) {
		memcpy(&coeffArray[0], &coeffs[0], sizeof(double) * numCoeffs);
	}

	double* getCoefficients() {
		return &coeffArray[0];
	}

	double* getStateArray() {
		return &stateArray[0];
	}

protected:
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double stateArray[numStates] = { 0.0, 0.0, 0.0, 0.0 };
};


struct LB_LPFParameters {
	LB_LPFParameters() {}

	LB_LPFParameters& operator=(const LB_LPFParameters& params) {
		if (this == &params) return *this;
		fc = params.fc;
		Q = params.Q;
		return *this;
	}

	double fc = 100;
	double Q = 0.707;
};

struct LB_PEQParameters {
	LB_PEQParameters() {}

	LB_PEQParameters& operator=(const LB_PEQParameters& params) {
		if (this == &params) return *this;
		fc = params.fc;
		Q = params.Q;
		gain = params.gain;
		return *this;
	}

	double fc = 1000;  
	double Q = 0.707;  
	double gain = 0.0; 
};

struct LB_HSFParameters {
	LB_HSFParameters() {}

	LB_HSFParameters& operator=(const LB_HSFParameters& params) {
		if (this == &params) return *this;
		fc = params.fc;
		gain = params.gain;
		return *this;
	}

	double fc = 1000;
	double gain = 0.0;
};

struct LB_CompressorParameters {
	LB_CompressorParameters() {}
	
	LB_CompressorParameters& operator=(const LB_CompressorParameters& params) {
		if (this == &params) return *this;
		threshold_dB = params.threshold_dB;
		ratio = params.ratio;
		attackTime = params.attackTime;
		releaseTime = params.releaseTime;
		outputGain = params.outputGain;
		return *this;
	}

	double threshold_dB = threshold_dB;
	double ratio = ratio;
	double attackTime = attackTime; //in ms
	double releaseTime = releaseTime;
	double outputGain = outputGain;

};

struct LB_EnvDetectorParameters {
	LB_EnvDetectorParameters() {}
	LB_EnvDetectorParameters& operator=(const LB_EnvDetectorParameters& params) {
		if (this == &params) return *this;
		attackTime = params.attackTime;
		releaseTime = params.releaseTime;
		detect_dB = params.detect_dB;
		return *this;
	}

	double attackTime = 0.0; //in ms
	double releaseTime = 0.0; //in ms
	bool detect_dB = false;
};

/* 
	Low Pass Filter Object, by Lucas Burkholder
*/

class LB_LPF {
public:
	LB_LPF() {}
	~LB_LPF() {}

	LB_LPFParameters getParameters() {
		return parameters;
	}

	void setParameters(LB_LPFParameters _parameters) {
		if (parameters.fc != _parameters.fc || parameters.Q != _parameters.Q) {
			parameters = _parameters;
		}
		else return;

		if (parameters.Q <= 0) parameters.Q = 0.707;

		//update coefficients
		calculateFilterCoeffs();
	}

	bool reset(double _sampleRate) {
		sampleRate = _sampleRate;
		return biquad.reset(sampleRate);
	}

	double processAudioSample(double xn);

	void setSampleRate(double _sampleRate) {
		sampleRate = _sampleRate;
		calculateFilterCoeffs();
	}

	bool canProcessAudioFrame() { return false; }

protected:
	LBBiquad biquad;
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	LB_LPFParameters parameters;
	double sampleRate = 48000;

	bool calculateFilterCoeffs();
};

class LB_PEQ {
public: 
	LB_PEQ() {}
	~LB_PEQ() {}

	LB_PEQParameters getParameters() {
		return parameters;
	}

	void setParameters(LB_PEQParameters _parameters) {
		if (parameters.fc != _parameters.fc || parameters.Q != _parameters.Q || parameters.gain != _parameters.gain) {

			parameters = _parameters;
		} else return;

		if (parameters.Q <= 0) parameters.Q = 0.707;

		//update coefficients
		calculateFilterCoeffs();
	}

	bool reset(double _sampleRate) {
		sampleRate = _sampleRate;
		return biquad.reset(sampleRate);
	}

	double processAudioSample(double xn);

	void setSampleRate(double _sampleRate) {
		sampleRate = _sampleRate;
		calculateFilterCoeffs();
	}

	bool canProcessAudioFrame() { return false; }

protected:
	LBBiquad biquad;
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	
	LB_PEQParameters parameters;
	double sampleRate = 44100;


	bool calculateFilterCoeffs();
};

class LB_HSF {
public:
	LB_HSF() {}
	~LB_HSF() {}

	LB_HSFParameters getParameters() {
		return parameters;
	}

	void setParameters(LB_HSFParameters _parameters) {
		if (parameters.fc != _parameters.fc || parameters.gain != _parameters.gain) {
			parameters = _parameters;
		}
		else return;
		
		calculateFilterCoeffs();
	}

	bool reset(double _sampleRate) {
		sampleRate = _sampleRate;
		return biquad.reset(sampleRate);
	}

	double processAudioSample(double xn);

	void setSampleRate(double _sampleRate) {
		sampleRate = _sampleRate;
		calculateFilterCoeffs();
	}

	bool canProcessAudioFrame() { return false; }

protected:
	LBBiquad biquad;
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	LB_HSFParameters parameters;
	double sampleRate = 44100;
	bool calculateFilterCoeffs();
	
};

class LB_EnvDetector {
public:
	LB_EnvDetector() {}
	~LB_EnvDetector() {}

	LB_EnvDetectorParameters getParameters() {
		return parameters;
	}

	void setParameters(LB_EnvDetectorParameters _parameters) {
		parameters = _parameters;

		setAttackTime(_parameters.attackTime, true);
		setReleaseTime(_parameters.releaseTime, true);
	}

	virtual bool reset(double _sampleRate) {
		sampleRate = _sampleRate;
		lastEnvelope = 0.0;
		return true;
	}

	virtual double processAudioSample(double xn) {
		//full wave rectification
		double input = fabs(xn);

		//Square it (RMS)
		input *= input;

		double currEnvelope = 0.0;

		if (input > lastEnvelope)
			currEnvelope = attackTime * (lastEnvelope - input) + input;
		else
			currEnvelope = releaseTime * (lastEnvelope - input) + input;

		//bound here if desired

		currEnvelope = fmax(currEnvelope, 0.0);

		lastEnvelope = currEnvelope;

		//do SQRT bc RMS
		currEnvelope = pow(currEnvelope, 0.5);

		if (!parameters.detect_dB) return currEnvelope;

		return 20.0 * log10(currEnvelope);
	}

	virtual void setSampleRate(double _sampleRate) {
		if (sampleRate == _sampleRate) return;
		sampleRate = _sampleRate;

		setAttackTime(parameters.attackTime, true);
		setReleaseTime(parameters.releaseTime, true);
	}

	virtual bool canProcessAudioFrame() { return false; }

protected:
	LB_EnvDetectorParameters parameters;
	double sampleRate = 44100;
	double attackTime;
	double releaseTime;
	double lastEnvelope;

	void setAttackTime(double attack_ms, bool forceCalc) {
		if (!forceCalc && parameters.attackTime == attack_ms) return;
		parameters.attackTime = attack_ms;
		//try this with -1?
		attackTime = exp(TLD_AUDIO_ENVELOPE_ANALOG_TC / (attack_ms * sampleRate * 0.001));
	}

	void setReleaseTime(double release_ms, bool forceCalc) {
		if (!forceCalc && parameters.releaseTime == release_ms) return;
		parameters.releaseTime = release_ms;
		releaseTime = exp(TLD_AUDIO_ENVELOPE_ANALOG_TC / (release_ms * sampleRate * 0.001));
	}
};

class LB_Compressor {
public:
	LB_Compressor() {}
	~LB_Compressor() {}

	LB_CompressorParameters getParameters() {
		return parameters;
	}

	void setParameters(LB_CompressorParameters _parameters) {
		parameters = _parameters;

		LB_EnvDetectorParameters detectorParams = detector.getParameters();
		detectorParams.attackTime = parameters.attackTime;
		detectorParams.releaseTime = parameters.releaseTime;
		detector.setParameters(detectorParams);
	}

	virtual bool reset(double _sampleRate) {
		detector.reset(_sampleRate);
		LB_EnvDetectorParameters detectorParams = detector.getParameters();
		detectorParams.detect_dB = true;
		detector.setParameters(detectorParams);
		return true;
	}

	virtual double processAudioSample(double xn) {
		double detect_dB = detector.processAudioSample(xn); // no sidechain here yet

		//compute gain
		double gr = computeGain(detect_dB);

		//makeup gain
		double makeupGain = pow(10.0, parameters.outputGain / 20.0);
		return xn * gr * makeupGain;
	}

	virtual bool canProcessAudioFrame() { return false; }

protected:
	LB_CompressorParameters parameters;
	double sampleRate = 44100;

	LB_EnvDetector detector;

	inline double computeGain(double detectorLevel_dB) {
		double output_dB = 0.0;

		if (detectorLevel_dB <= parameters.threshold_dB)
			output_dB = detectorLevel_dB;
		else
			output_dB = parameters.threshold_dB + ((detectorLevel_dB - parameters.threshold_dB) / parameters.ratio);

		double gainReduction_dB = output_dB - detectorLevel_dB; //negative value if reducing gain

		return pow(10.0, gainReduction_dB / 20.0);
	}
};



/**
TanH wave shaper
saturation parameter decides how much to saturate
*/
inline double tanhWaveShaper(double xn, double saturation)
{
	return tanh(saturation*xn) / tanh(saturation);
}