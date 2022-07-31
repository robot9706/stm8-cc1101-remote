# Custom remote firmware

This firmware acomplishes a very simple task. It transmits 4 predefined codes as long as one of the 4 buttons on the remote is being held. Otherwise it puts everything to sleep to conserve battery.

## Compile and flash

The firmware uses SDCC to compile and STVP (ST Visual Programmer) is used to flash the STM8 chip.

Compiling can be done using `build.bat`

Flashing can be done using the GUI of STVP or the following commandline:

```
STVP_CmdLine.exe -no_loop -BoardName=ST-LINK -ProgMode=SWIM -Device=STM8L15xG4 -fileProg="build\remote.hex"
```

## How the firmware works

When the CPU boots up (by being powered from a battery) it sets up all the hardware (CPU registers, SPI, TIM1, GPIOs, IRQs), then it resets the CC1101 radio chip and the main loop starts.

// TODO: Main loop

If none of the buttons are being pressed and there is no more data to be transmitted, the CPU puts the CC1101 to low power mode, disables its peripherals and halts until a button interrupts wakes it up.