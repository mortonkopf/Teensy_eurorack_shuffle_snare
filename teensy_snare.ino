/*
https://www.soundonsound.com/techniques/synthesizing-drums-snare-drum

TODO - use rising edge instead of voltage > 250mV
lose ratchet but gain single hit

*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthNoiseWhite     noise1;         //
AudioSynthWaveform       waveform4;      //sine 180Hz
AudioSynthWaveform       waveform3;      //sine 330Hz
AudioSynthWaveform       waveform1;      //triangle 175Hz
AudioSynthWaveform       waveform2;      //triangle 224Hz
AudioFilterStateVariable filter1;        // cut off filter for noise
AudioMixer4              mixer2;         // two sine waves together
AudioMixer4              mixer1;         // two triangles together
AudioFilterBiquad        biquad1;        // use as notch pass filter
AudioAmplifier           amp1;           // triangles amp (use contoured input)
AudioAmplifier           amp2;           // sines amp (use contoured input)
AudioAmplifier           amp3;           // noise amp (contour controlled)
AudioMixer4              mixer3;         // noise plus sine plus triangle
AudioEffectEnvelope      envelope1;      //shape the hihat sound
AudioOutputAnalog        dac1;           // final output
AudioConnection          patchCord1(noise1, 0, filter1, 0);
AudioConnection          patchCord2(waveform4, 0, mixer2, 1);
AudioConnection          patchCord3(waveform3, 0, mixer2, 0);
AudioConnection          patchCord4(waveform1, 0, mixer1, 0);
AudioConnection          patchCord5(waveform2, 0, mixer1, 1);
AudioConnection          patchCord6(filter1, 0, biquad1, 0);
AudioConnection          patchCord7(mixer2, amp2);
AudioConnection          patchCord8(mixer1, amp1);
AudioConnection          patchCord9(biquad1, amp3);
AudioConnection          patchCord10(amp1, 0, mixer3, 0);
AudioConnection          patchCord11(amp2, 0, mixer3, 1);
AudioConnection          patchCord12(amp3, 0, mixer3, 2);
AudioConnection          patchCord13(mixer3,envelope1);
AudioConnection          patchCord14(envelope1, dac1);

//need to add in an envelop


// GUItool: end automatically generated code

int Osc1 = 175;
int Osc2 = 224;
int Osc3 = 330;
int Osc4 = 180;


//-----------------------inputs outputs
int closedLeng = A9;//rotary to control lenth of close hihat
int openLeng = A8;//rotary to control length of open hat
int reson = A7;//resonance / filter sweep
int trigOpenSnare=A5;
int trigClosedSnare=A4;
int LedOpen = 17; //closed trig indicator pin for led
int LedClosed =16;

//------for receiving triggers--------
unsigned long currentClTime = 0;
unsigned long oldClTime = 0;
unsigned long clockGap = 50; //gap for counting only one trigger per pin high
unsigned long currentClTime2 = 0;
unsigned long oldClTime2 = 0;
unsigned long clockGap2 = 50; //change to 50 after debug

float sweepRead=0;
float sweepVal;

int closedRead=0;
int closedHold =0;
int openRead=0;
int openHold =0;

//-------indication leds
unsigned long newLedTime = millis();
unsigned long ledTime = 0;
unsigned long oldLedTime = 0;
//-------indication leds
unsigned long newLedTime2 = millis();
unsigned long ledTime2= 0;
unsigned long oldLedTime2 = 0;




//-------------------------------setup------------//
void setup() {
//------provide CPU space
  AudioMemory(50);
//dac1.analogReference(EXTERNAL);// may need to remove this re noise...
  
// ------ configure the six waveforms ----
  waveform1.frequency(Osc1);
  waveform2.frequency(Osc2);
  waveform3.frequency(Osc3);
  waveform4.frequency(Osc4);

  
  waveform1.amplitude(1);
  waveform2.amplitude(1);
  waveform3.amplitude(1);
  waveform4.amplitude(1);

//---config the noise-----
  noise1.amplitude(1.0);//zero to 1
//--------config the filters--------
filter1.frequency(13000); //starting value
filter1.resonance(1.0);
//top and bottom end of filter >3000 to 10000??
//Set the filter's resonance. Q ranges from 0.7 to 5.0. 
//Resonance greater than 0.707 will amplify the signal near the corner frequency. 

//setNotch(stage, frequency, Q);
//Configure one stage of the filter (0 to 3) with band reject (notch) response. 
//Q controls the width of rejected frequencies.

biquad1.setNotch(0, 7000, 0.707);// notch frequency variable

//------------config inital evelope settings-----
envelope1.delay(0);//0
envelope1.attack(0);//20
envelope1.hold(10);//1
envelope1.decay(30);//10
envelope1.sustain(0.0);//0.0
envelope1.release(0.2);//0.2


//----------------config the mixers
  mixer1.gain(0, 1.0);
  mixer1.gain(1, 1.0);
  mixer2.gain(0, 1.0); 
  mixer2.gain(1, 1.0);
  mixer3.gain(0, 1.0); 
  mixer3.gain(1, 1.0);
  mixer3.gain(2, 1.0);

  amp1.gain(1.0);

//---------------set the six waveforms all to Square
//    waveform4.begin(WAVEFORM_SQUARE);
    waveform1.begin(WAVEFORM_TRIANGLE);
    waveform2.begin(WAVEFORM_TRIANGLE);
    waveform3.begin(WAVEFORM_SINE);
    waveform4.begin(WAVEFORM_SINE);


//-------dont forget to set the pinmodes
 pinMode(closedLeng, INPUT); //rotary forclock division selection
 pinMode(openLeng, INPUT); 
 pinMode(reson, INPUT);
 pinMode(trigOpenSnare, INPUT_PULLDOWN);
 pinMode(trigClosedSnare, INPUT_PULLDOWN);
 pinMode(LedOpen, OUTPUT);
 pinMode(LedClosed, OUTPUT);
 
}//----------end of setup----------------//


void loop() {
 
  //-------switch of indicator Leds if on for long enough
  newLedTime = millis();
  if (newLedTime > (ledTime + 20)) {
    digitalWrite(17, LOW);
  }
  newLedTime2 = millis();
  if (newLedTime2 > (ledTime2 + 20)) {
    digitalWrite(16, LOW);
  }


  sweep();//continuously read sweep value
  
//-------check for triggers
 if (analogRead(trigClosedSnare) > 250) { //250mV reading means trigger voltage
     tightSnare();  
 }
 if (analogRead(trigOpenSnare) > 250) {
     openSnare();  
 } 

}//--------------end of loop-------------//


void tightSnare() {
  //if a closedHat input is registered, is it the same one?,
  currentClTime = millis();
 if (currentClTime > (oldClTime + clockGap)){
  
  oldClTime = currentClTime;
  digitalWrite(16, HIGH);
  ledTime2 = millis();
  closedRead = analogRead(closedLeng) + 1; // value is 0-1023
  closedHold = map(closedRead, 0, 1023, 50, 170); // map to range of 1-250
  envelope1.decay(closedHold);
    AudioNoInterrupts();
    envelope1.noteOn();
    AudioInterrupts();
  }
  else{ 
  }
}

void openSnare() {
  currentClTime2 = millis();
 if (currentClTime2 > (oldClTime2 + clockGap2)){
  
  oldClTime2 = currentClTime2;
  digitalWrite(17, HIGH);
  ledTime = millis();
  openRead = analogRead(openLeng) + 1; // value is 0-1023
  openHold = map(openRead, 0, 1023, 80, 300);
  envelope1.decay(openHold);
    AudioNoInterrupts();
    envelope1.noteOn();
    AudioInterrupts();
 }
 else{
 }
}


void sweep() {
  sweepRead = analogRead(reson) + 1; // value is 0-1023
  sweepVal = map(sweepRead, 0, 1023, 1000, 15000);//top and bottom end of filter >3000 to 10000??
 // filter1.frequency(sweepVal);
  biquad1.setNotch(0, sweepVal, 2000/sweepVal/*0.707*/);
}
