# STM32F446_Lock_in_Implementation
Implementation of a digital lock-in amplifier using a STM32F446RE Nucleo Board

This Tutorial describes the steps of develop of a digital lock-in amplifier.

## **Theory Of Operation**
The lock-in amplifier is an instrument based on the principle called synchronous detection.
Its purpose is to recover small or weak signals that would otherwise be lost in noise. This device serves to detect the amplitude of a signal s(t) that is overlaid by noise. The instrument can be considered as a highly selective amplifier that operates as a bandpass filter, whose center frequency is determined by the reference signal r(t). The lock-in amplifier serves to extract the signal from noise.

When a measurement is being taken and the signal amplitude is large enough to be detected by a multimeter or oscilloscope, it is not necessary to use a lock-in amplifier. In these cases, simply use a multimeter and the problem is solved. On the other hand, when dealing with small signals, whose noise may be greater than the signal itself, or when great precision is desired, the lock-in amplifier is the right instrument for this measurement.

***But, what does the lock-in amplifier do exactly?***

The figure below illustrates the basic operation of a lock-in amplifier.
![Lock-in Processing img](https://user-images.githubusercontent.com/114233216/228120262-0e5bbd8e-1b59-4e33-bd7d-913f8ae0ed24.png)

In summary, the signals Vs and Vr are multiplied in phase and in quadrature (i.e. with Vr shifted by 90°), are filtered, and finally undergo vector multiplication. 

- But, Where do these Vs and Vr signals come from? 
- How does this make the measurement more accurate and eliminate noise? 

The figure below explain these signals:

![Fig1](https://user-images.githubusercontent.com/114233216/228856030-cec655f7-6a26-45dd-9ce9-a01b09bf685f.png)

The signal to be measured is the signal s(t) obtained at the output of the experiment. The r(t) signal is a sinusoidal signal used to modulate an experiment. At the output of the experiment, the result is the interest signal s(t), also sinusoidal, but contaminated with the noise inherent to the environment. Thus, s(t) and r(t) have fundamentally the same frequency. In synchronous detection, when s(t) and r(t) are multiplied, the noise present in s(t) is also multiplied together, however, the synchronous detection process (multiplication + filtering) only sees the part of s(t) which has the same frequency as r(t), so that noise and unwanted components [components with other frequencies that may contaminate the signal s(t)] are eliminated. At the output of the synchronous detection block (OUT), the result is a value referring to the amplitude of s(t), but free of noise.


* *Harmonic Component Detection*

Digital lock-in amplifiers have the ability to detect harmonic components of the signal of interest. This is interesting in some types of experiments that have a quadratic behavior. In these cases, when modulating the experiment with a sinusoidal reference signal of frequency f, the signal at the output of the experiment will have a component with the same frequency f, in addition to components with 2f, 3f, 4f or more. WMS (Wavelength Modulation Spectroscopy) experiments have this type of behavior. In these experiments, it is interesting to detect the value of the second harmonic component at the output of the experiment. This is one of the reasons why lock-in is an obligatory instrument in these types of experiments.


The entire mathematical operation of lock-in is described in detail in this file:
[Lock-in Processing.pdf](https://github.com/BernardoLima92/STM32F446_Lock_in_V01/files/11084935/Lock-in.Processing.pdf).


***A more didactic way to understand Lock-in:***

Wheel, the most part of describe above was based in the very well founded theory founded in the books from Meade (Lock-In Amplifiers: Principles and Applications) and Gerhard Kloos (Applications of Lock-in Amplifiers in Optics) about the operation/application of the lock-in amplifier. The objective here is not to bring the theory in a raw form. In this tutorial I will try to explain in a simpler and more didactic way the operation of the lock-in amplifier.

When we are in graduation and start to work with arduino or other microcontrollers, one of the activities carried out is the temperature measurement with ICs like the LM35. In this case, the microcontroller's ADC is used to read an analog signal that is proportional (or nearly proportional) to the ambient temperature.
So, the excited student creates a program to measure the temperature and print the value on an LCD display. In many situations what happens is that the temperature value varies slightly. For example, sometimes it shows 26°C/27°C, and it varies between these two values, which are very close. This can happen for several reasons: the value is close to the resolution limit, noise, poor contact (there are several possible reasons). But the student doesn't want to know the reason, he just wants to solve the problem and make the temperature stop varying, stay stable. And then the student has the idea that solves the problem: take an average of 100 readings and use the average value as the correct temperature value.
The student realizes that taking an arithmetic average of n readings solves the problem.

One way to understand the lock-in amplifier is to know that it does
exactly the same thing: it does an average. However, not simply an average equal to what was done by the student, because in lock-in, the signal is alternating and periodic. Lock-in uses (usually) a sinusoidal signal called the reference signal r(t) to modulate the experiment, making the signal you want to measure, which was purely DC, now have an AC component. The average is made with a simple integration process, and the value obtained is the DC value that would initially have if the signal were not modulated, however, free of noise.

Instead of averaging the DC readings, the lock-in averages the AC readings after synchronous detection. This simple change is the differential that causes the noise to be eliminated.

* *What do you mean "CA read after synchronous detection"?*

Think about it, if the lock-in only averaged the sinusoidal signal read from the ADC port, nothing special would be done. Most of the noise that was contaminating the signal would also be present in the result of this AC average. Thus, the lock-in averages only those components that have the same frequency as the reference frequency used to modulate the signal. This causes unwanted components and noise to be eliminated. So it is correct to say that the lock-in also works as a highly selective band-pass filter.

The idea of averaging the AC signal, not the DC signal, is because at low frequencies there is a strong influence of noise called 1/f. As the frequency increases, the intensity of the noise decreases. This is the foundation of the lock-in amplifier.


## **Building the Lock-in With STM32**


We can divide the construction of the lock-in amplifier into two major steps:
- Step 1: Acquisition of the Vs and Vr signals.
- Step 2: Multiplication, filtering, vector multiplication, printing the result.

## **STEP 1**:

In practical terms, the signal Vs is the signal that we are actually interested in measuring, and it originates from some experiment. The reference signal is a signal generated by an oscillator, either externally or internally.
Note: a basic requirement for the operation of a lock-in amplifier is that the signals of interest and reference signals maintain a fixed phase difference during the entire operation, that is, the signals must be synchronized.

When the reference signal is generated externally, by a bench function generator for example, the lock-in amplifier must be able to acquire (read in its analog port) the Vs and Vr signals synchronously. This requires on the lock-in side the use of some circuitry called PLL (Phased Lock Loop). 

On the other hand, when the reference signal is generated internally, the lock-in amplifier only needs to acquire (read in its analog port) the Vs signal, because the reference signal is already stored in the lock-in's own memory. In this way it is much easier to synchronize the signals and to shift the reference signal with great precision.

This tutorial is based on the internally generated reference signal in the STM32 microcontroller itself.

The generation of a sinuisodal signal that serves as reference is done using a technique called DDS (Direct Digital Synthesis), which uses a look-up table (LUT) that has the discrete values of the signal to be generated, a DAC and a timer to control the operation

The figure below illustrates the basic application of this technique.
![DDS](https://user-images.githubusercontent.com/114233216/228123336-85b45063-5246-4f05-8874-0763f3bbaa47.png)
By continuously putting into the DAC the values of the table LUT (P0, P1, P2, P3, P4...P9), what we will have at the output is a sinusoidal signal. The frequency of this signal depends on the speed at which the DAC value is updated. The formula shown in the figure below allows you to determine the frequency of the generated signal:

![fórmula](https://user-images.githubusercontent.com/114233216/233111786-14b10610-ff40-491e-a723-a08018573247.png)


where ARR is the timer count limit value and Ns is the number of values in the LUT.

In this work TIMER 8, DAC1, ADC0, DMA1 were used. For The sinusoidal signal generated in this work a LUT with 128 positions was created, the ARR value was set to 4689, and the operating frequency of the chip was set to 120M Hz. The result was a sinusoidal signal of 199.89 Hz.

But for lock-in processing, in addition to generating the reference signal (Vr), it is also necessary to acquire the signal of interest (Vs), and in a synchronized manner. So, what the micrcontroller-based lock-in does is repeatedly:
- update the DAC value with the next LUT value;
- Read the analog port;

The Figure below illustrates how this operation is done so that the Vs and Vr signals are synchronized:
![timediagram](https://user-images.githubusercontent.com/114233216/233112516-aa11e94c-c86f-48ac-9f24-dc1ffb43ad32.png)


The TIMER 8 counts from 0 to ARR. Two triggers are configured:
- When the timer passes 1500, an analog reading is taken and its result is stored in a vector.
- When the timer reaches the value of ARR (Overflow), the DAC value is updated with the next LUT value.

This whole process is performed by the chip's DMA, in order to leave the microcontroller free for the other necessary operations. This is done repeatedly. This ensures that the signals Vs and Vr are synchronized, i.e. have a fixed phase difference during the whole process.

Here at this point there is an important practical aspect that should be commented on. It is necessary to determine when complete periods of the analog signal will be read. This total number of periods is related to the integration time of the professional lock-in, and is directly related to the quality of the signal to be processed.
In this work, a vector with 1280 positions was created to store the values read from the analog port. That is, 100 complete periods of the Vs signal will be read.

After obtaining the 1280 values of Vs (which corresponds to 100 complete periods), the step 2 starts to be performed in lock-in processing with the integration (or low-pass filtering)

## **STEP 2**
Here what happens is basic mathematical operations, such as vector multiplication and arithmetic mean.

Before explaining the code, it is necessary to explain where the s(t) and r(t) signals are.
- The signal s(t) is represented by the vector AdcRead[1280];
- The r(t) signal is present on line 49 of the 2ndHarmonic.c file present in this repository. Such a signal has the name LUT[Ns]. It was created in excel with the function sin(2 * pi() * 8 * i/128), with i ranging from 0 to 127.

Our objective in this work is **to detect the 2nd harmonic** component in the experiment output. **So what we need to do is multiply the signal of interest s(t) by a modified version of the reference signal , which I will call rd(t)**. This rd(t) signal also has 128 points, however, it has twice the frequency of the signal used to generate the signal in the DAC. (Obs: If we wanted the fourth harmonic component, the signal rd(t) would have 4x the frequency of the signal r(t))

The code snippet below shows, in the first for loop, the creation of this signal during the multiplication process itself.
The second for loop performs low-pass filtering. In fact, what is done is a simple arithmetic average, which in practice has the same effect, since the signal obtained is sinusoidal.
The third and fourth loops do the same process, but the rd(t) signal is shifted by 90°.
In the last line of this section the vector calculation is performed.


```
// Second Harmonic Calculation in Phase
for (i = 0; i < (Periodos*Ns); i++){
		resultado[i] = (AdcRead[i] * sin(arg2*i/Ns) );
	}
for (i = 0; i < (Periodos*Ns); i++){
		soma = soma + resultado[i];
	}
	fase2h[j] =  soma/(Periodos*Ns);			//This is the U1 result
	soma = 0;

// Second Harmonic Calculation in Quadrature
for (i = 0; i < (Periodos*Ns); i++){
		resultado[i] = (AdcRead[i] * cos(arg2*i/Ns) );
	}
for (i = 0; i < (Periodos*Ns); i++){
		soma = soma + resultado[i];
	}
	quad2h[j] =  soma/(Periodos*Ns);			//This is the U2 result
	soma = 0;

// Calculation of the Second Harmonic modulus
mod2h[j] = pow( (pow(fase2h[j],2) + pow(quad2h[j], 2)) , 0.5);	// This is the M result	

```


## **Prints of developed Software on STM32CubeIDE**

1. PINOUT
![Fig1 - PINOUT](https://user-images.githubusercontent.com/114233216/233118030-64949013-482a-4d8f-b180-eca9a746f4b6.png)


2. CLOCK CONFIGURATION
![Fig2 - CLOCK CONFIGURATION](https://user-images.githubusercontent.com/114233216/233118068-d49a0a5d-6d8a-4a8e-a257-46ddefa1b9a9.png)


3. ADC1
![Fig3 - ADC1](https://user-images.githubusercontent.com/114233216/233118108-03929909-1c38-4e64-b86f-8de144c18b13.png)


4. ADC1 (Part of DMA)
![Fig4 - ADC1 (dma)](https://user-images.githubusercontent.com/114233216/233118158-7876e620-8d0c-4a4f-aa97-6901093ab899.png)


5. DAC
![Fig5 - DAC](https://user-images.githubusercontent.com/114233216/233118191-ff53f195-2dd4-4f44-98d0-c8f490b57832.png)


6. DAC (Part of DMA)
![Fig6 - DAC (dma)](https://user-images.githubusercontent.com/114233216/233118212-98723b67-5272-40e7-84cc-0abc98967c73.png)


7. TIMER8
![Fig7 - TIM8](https://user-images.githubusercontent.com/114233216/233118246-5b56e15d-a20f-4559-988e-5e81ead305cd.png)


8. TIMER8
![Fig8 - TIM8](https://user-images.githubusercontent.com/114233216/233118294-9e5baf79-9103-4aa9-8df4-8c2102c8d48a.png)


9. USART
![FiG9 - USART](https://user-images.githubusercontent.com/114233216/233118318-957bf6eb-612d-453e-a0ff-864661c2a0ef.png)


10. NVIC
![Fig10 - NVIC](https://user-images.githubusercontent.com/114233216/233118343-21554783-98bd-4613-8c5b-ff14a9903819.png)



## **WMS Experiment - Comparison between STM32 based lock-in and Signal Revocery DSP7265 lock-in**


