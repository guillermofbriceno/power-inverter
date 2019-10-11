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
#include "spwm_table.h"

#define SWPWM_PIN1 GPIO_PIN_6
#define SWPWM_PIN2 GPIO_PIN_7
#define SWPWM_PORT GPIO_PORTB_BASE
#define SWPWM_PERIPH SYSCTL_PERIPH_GPIOB

#define RAMP_FREQ 60000 //Hz
#define MCU_FREQ 80000000
#define DEADBAND 8

extern const double pwmCycles[];
const uint32_t arr_size = 1000;
volatile uint32_t arr_index = 0;

//(1/f) * k = s

void initGPIO() {
	SysCtlPeripheralEnable(SWPWM_PERIPH);
	while(!SysCtlPeripheralReady(SWPWM_PERIPH));
}

void initPWM() {
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

void timer0IntHandler() {
	uint32_t status = 0;
	status = TimerIntStatus(TIMER0_BASE, true);
	TimerIntClear(TIMER0_BASE, status);

	if (arr_index > arr_size) {
		arr_index = 0;
	}
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pwmCycles[arr_index]);

	arr_index += 1;
}

void initPWMTimer(uint32_t periodCycles) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerIntRegister(TIMER0_BASE, TIMER_A, timer0IntHandler);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerLoadSet(TIMER0_BASE, TIMER_A, periodCycles - 1);
	TimerEnable(TIMER0_BASE, TIMER_A);
}

int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	initGPIO();
	initPWM();

	uint32_t sawPeriodCycles = MCU_FREQ / RAMP_FREQ; //period of sawtooth in cycles
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, sawPeriodCycles);

	initPWMTimer(sawPeriodCycles);

	while(true) {}
}
