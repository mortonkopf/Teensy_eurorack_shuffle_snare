# Teensy_eurorack_snare
code using the Teensy DSP tool to generate a four wave plus noise Snare triggered by CV input

based on the description of snare synthesis found here:
https://www.soundonsound.com/techniques/synthesizing-drums-snare-drum

using the four waveforms and noise described in the above:
AudioSynthNoiseWhite     noise1;         // 
AudioSynthWaveform       waveform4;      //sine 180Hz 
AudioSynthWaveform       waveform3;      //sine 330Hz 
AudioSynthWaveform       waveform1;      //triangle 175Hz 
AudioSynthWaveform       waveform2;      //triangle 224Hz 

then using cutoff filter and notch filter.
