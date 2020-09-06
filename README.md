# rpi-fan

Turns a fan on and off to keep the CPU at a target temperature. It uses a GPIO pin to manage the fan. Written in C to minimize CPU overhead.

## Usage
rpi-fan PIN TEMPERATURE DELAY

PIN - the GPIO pin that the fan is connected to

TEMPERATURE - the target CPU temperature in Celsius

DELAY - time between temperature checks in milliseconds
