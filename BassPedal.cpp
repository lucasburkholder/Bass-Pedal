#include <string.h>

#include "daisy_seed.h"
#include "FXObjects/LBFX.h"
#include "FXObjects/LBFX.cpp"
#include "daisysp.h"
#include "FXObjects/BassPedalFX.h"

using namespace daisy;

DaisySeed hw;
LB_EnvDetector inputLevelDetector;
FatPunch fatPunch;
MelodyMode melodyMode;
Switch fatButton, darkButton, punchButton, melodyButton;
Led fatLED, darkLED, punchLED, melodyLED;
RgbLed inLevelLED;

enum fpModeNum {fat, darken, punchComp};
bool fpModes[3] = {false, false, false};
bool prevFatButtonState, prevDarkButtonState, 
    prevPunchButtonState, prevMelodyButtonState = false;


static void Callback(AudioHandle::InterleavingInputBuffer  in,
                     AudioHandle::InterleavingOutputBuffer out,
                     size_t                                size)
{
    //size is buffer size (# of samples in buffer)
    // Read input level knob value
    float knobVal = hw.adc.GetFloat(0);
    float inputGain_dB = knobVal  * 84.0 - 60.0; //map knobVal 0-1 to dB gain amount -60dB to +24dB

    // Debounce buttons
    fatButton.Debounce();
    darkButton.Debounce();
    punchButton.Debounce();
    melodyButton.Debounce();

    // Update FX Object parameters. Button dictates fpParams param, which dictates LED state
    FatPunchParameters fpParams = fatPunch.getParameters();
    fpParams.fatOn = (fatButton.Pressed() && !prevFatButtonState) ? !fpParams.fatOn : fpParams.fatOn;
    fpParams.darkenOn = (darkButton.Pressed() && !prevDarkButtonState) ? !fpParams.darkenOn : fpParams.darkenOn;
    fpParams.punchCompOn = (punchButton.Pressed() && !prevPunchButtonState) ? !fpParams.punchCompOn : fpParams.punchCompOn;
    fpParams.inDistAmt = 1.0; 
    fatPunch.setParameters(fpParams);

    //Set melody mode object parameter based on melody button
    MelodyModeParameters mmParams = melodyMode.getParameters();
    mmParams.on = (melodyButton.Pressed() && !prevMelodyButtonState) ? !mmParams.on : mmParams.on;
    melodyMode.setParameters(mmParams);

    //turn everything else off if melody mode is on
    if (mmParams.on) {
        fpParams.fatOn = false;
        fpParams.darkenOn = false;
        fpParams.punchCompOn = false;
    }

    //Set LEDs based on mode
    fatLED.Set(float(fpParams.fatOn));
    darkLED.Set(float(fpParams.darkenOn));
    punchLED.Set(float(fpParams.punchCompOn));
    melodyLED.Set(float(mmParams.on));

    //Update LEDs
    fatLED.Update();
    darkLED.Update();
    punchLED.Update();
    melodyLED.Update();
    inLevelLED.Update(); 

    // AUDIO PROCESSING
    double inputSample, inputLevel;
    float inputLevelSum = 0.0;
    for (size_t i = 0; i < size; i += 2) {
        //Apply input gain
        inputSample = in[i] * knobVal * 5;

        // Read input level, output to rgb LED
        inputLevel = inputLevelDetector.processAudioSample(inputSample); 
        if (i == 0)
            inLevelLED.SetColor(getLEDColor(inputLevel));

        // Process audio through fatPunch object
        out[i] = fatPunch.processAudioSample(inputSample);
        
        out[i] = melodyMode.processAudioSample(out[i]);
        out[i+1] = out[i]; //interleaved output
    } 

    prevFatButtonState = fatButton.Pressed();
    prevDarkButtonState = darkButton.Pressed();
    prevPunchButtonState = punchButton.Pressed();
    prevMelodyButtonState = melodyButton.Pressed();
}

int main(void)
{
    //Initialize hardware board
    float sampleRate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sampleRate = hw.AudioSampleRate();

    //Initialize LEDs
    fatLED.Init(hw.GetPin(7), false);
    darkLED.Init(hw.GetPin(8), false);
    punchLED.Init(hw.GetPin(9), false);
    melodyLED.Init(hw.GetPin(10), false);
    inLevelLED.Init(hw.GetPin(13), hw.GetPin(12), hw.GetPin(11), false);

    //Initialize input level detector
    inputLevelDetector.reset(sampleRate);
    LB_EnvDetectorParameters inDetectorParams = inputLevelDetector.getParameters();
    inDetectorParams.attackTime = 50.0;
    inDetectorParams.releaseTime = 50.0;
    inDetectorParams.detect_dB = true;
    inputLevelDetector.setParameters(inDetectorParams);

    //Initialize fatPunch object -- equivalent to [daisySP filter].init()
    fatPunch.reset(sampleRate);
    FatPunchParameters fatPunchParams;
    fatPunchParams.fatOn = false;
    fatPunchParams.darkenOn = false;
    fatPunchParams.punchCompOn = false;
    fatPunchParams.inDistAmt = fatPunchParams.punchCompOn ? 4.0 : 1.5; //currently these vals are NOT getting sent to the distortion function.
    fatPunch.setParameters(fatPunchParams);

    //Initialize melodyMode object -- equivalent to [daisySP filter].init()
    melodyMode.reset(sampleRate);
    MelodyModeParameters mmParams;
    mmParams.on = false;
    melodyMode.setParameters(mmParams);

    //Initialize knob
    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hw.GetPin(21));
    hw.adc.Init(&adcConfig, 1);
    hw.adc.Start();
    
    //Initialize buttons
    fatButton.Init(hw.GetPin(28), 1000);
    darkButton.Init(hw.GetPin(27), 1000);
    punchButton.Init(hw.GetPin(26), 1000);
    melodyButton.Init(hw.GetPin(25), 1000);
    
    hw.StartAudio(Callback);
    while(1) {}
}