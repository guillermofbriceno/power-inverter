// Define GPIO Ports
#define SWPWM_PIN1      GPIO_PIN_6              // PWM Non-Inverted Pin
#define SWPWM_PIN2 	GPIO_PIN_7              // PWM Inverted Pin
#define SWPWM_PORT 	GPIO_PORTB_BASE         // Port for both PWM pins
#define SWPWM_PERIPH 	SYSCTL_PERIPH_GPIOB     // PERIPH for PWM port

// Define generation params
#define RAMP_FREQ       60000                   // Freqency of sawtooth/ramp function function
#define SINE_FREQ       60                      // Sine wave frequency in Hz
#define MCU_FREQ        80000000                // Frequency of microcontroller (actually set by SysCtlClockSet)
#define DEADBAND        8                       // Amount of deadband in inverted PWM pin in cycles
#define PI              3.14159	                //
#define TIMESTEP        0.0000001               // Sampling timestep. Higher -> more precision
