#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"

#include "spwm_arraygen.h"
#include "macros.h"

extern volatile uint32_t        pwmCycles[1000];
extern const    uint32_t        arraySize;
volatile        uint32_t        arrayIndex = 0;
volatile        double          amplitude = MAX_AMP;
extern          void            generateArray(double percentAmplitude);

void initPeriph(uint32_t periph) {
        SysCtlPeripheralEnable(periph);
        while(!SysCtlPeripheralReady(periph));
}

void initPWM() {
        initPeriph(SWPWM_PERIPH);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
        //PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);
        GPIOPinConfigure(GPIO_PB6_M0PWM0);
        GPIOPinConfigure(GPIO_PB7_M0PWM1);
        GPIOPinTypePWM(SWPWM_PORT, SWPWM_PIN1);
        GPIOPinTypePWM(SWPWM_PORT, SWPWM_PIN2);
        PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 100);
        PWMDeadBandEnable(PWM0_BASE, PWM_GEN_0, DEADBAND, DEADBAND);
        PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);
}

void initMPPT() {
        //initPeriph(SYSCTL_PERIPH_ADC0);
        //initPeriph(SYSCTL_PERIPH_GPIOD);
        //GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);
        ////set the clock-

        //ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
        //ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
        //ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
        //ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
        //ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_TS | ADC_CTL_TE | ADC_CTL_END);
        //
        //ADCSeqenceEnable(ADC0_BASE, 1);
}

void timer0IntHandler() {
        uint32_t status = 0;
        status = TimerIntStatus(TIMER0_BASE, true);
        TimerIntClear(TIMER0_BASE, status);

        if (arrayIndex == arraySize) {
                arrayIndex = 0;
        }
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pwmCycles[arrayIndex]);

        arrayIndex += 1;
}

void initPWMTimer(uint32_t periodCycles) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
        TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
        TimerIntRegister(TIMER0_BASE, TIMER_A, timer0IntHandler);
        IntEnable(INT_TIMER0A);
        TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        IntMasterEnable();
        TimerLoadSet(TIMER0_BASE, TIMER_A, periodCycles - 1 + ADD_DELAY);
        TimerEnable(TIMER0_BASE, TIMER_A);
}

void trackIncrease() {
        if ((amplitude + AMP_STEP) >= MAX_AMP) {
                // ensure amplitude stays at MAX_AMP
                amplitude = MAX_AMP;
                return;
        } else {
                amplitude += AMP_STEP;
        }
}

void trackDecrease() {
        if ((amplitude - AMP_STEP) <= 0.0) {
                // ensure amplitude does not go below 0
               amplitude = 0.0; 
               return;
        } else {
                amplitude -= AMP_STEP;
        }
}

int main(void) {
        SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

        generateArray(0.98);
        initPWM();
        initMPPT();

        uint32_t sawPeriodCycles = MCU_FREQ / RAMP_FREQ; //period of sawtooth in cycles
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, sawPeriodCycles);

        initPWMTimer(sawPeriodCycles);

        while(true) {}
}
