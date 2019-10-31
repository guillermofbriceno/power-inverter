#include <math.h>
#include <stdint.h>
#include "macros.h"

volatile uint32_t pwmCycles[RAMP_FREQ / SINE_FREQ];
const uint32_t arr_size = RAMP_FREQ / SINE_FREQ;

double sawtooth(double x) {
	double slope = 2  * RAMP_FREQ;
	return (slope * x) - 1;
}

void generateArray(double percentAmplitude) {
	uint32_t samplesInRampPeriod = (1.0 / RAMP_FREQ) / TIMESTEP;
	uint32_t sawPeriodInClockCycles = MCU_FREQ / RAMP_FREQ;
	double periodElapsedTime = 0.0;
	double totalElapsedTime = 0.0;

	uint32_t samplesHigh = 0;
	uint32_t samplesLow = 0;
	uint32_t i;
	uint32_t j;

	for (i = 0; i < arr_size; i++) {
		for (j = 0; j < samplesInRampPeriod; j++) {
			double sine_t = percentAmplitude * sin(2 * PI * SINE_FREQ * totalElapsedTime);
			double saw_t = sawtooth(periodElapsedTime);

			if (sine_t > saw_t)
				samplesHigh++;
			else
				samplesLow++;

			periodElapsedTime = periodElapsedTime + TIMESTEP;
			totalElapsedTime = totalElapsedTime + TIMESTEP;
		}
		double dutyCycle = samplesHigh / (samplesHigh + samplesLow);
		uint32_t dutyCycleInClockCycles = dutyCycle * sawPeriodInClockCycles;
		pwmCycles[i] = dutyCycleInClockCycles;

		periodElapsedTime = 0.0;
		samplesHigh = 0;
		samplesLow = 0;
	}
}
