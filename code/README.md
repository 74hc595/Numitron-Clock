# IV-19 Numitron Clock Firmware

### Requirements
- GNU `arm-none-eabi-gcc` toolchain
- `lpc21isp` v1.97 (available from Homebrew on a Mac or from [GitHub](https://github.com/capiman/lpc21isp))
- 5V FTDI USB-serial cable

### To build
- Run `make`.
- Connect the FTDI cable to the 6-pin header on the clock (black wire indicated by a pointing triangle).
- Power on the clock while holding top button (SW1). Display should not illuminate.
- Run `make flash`.

### Hardware
The clock uses the [NXP LPC811](https://www.nxp.com/products/processors-and-microcontrollers/arm-based-processors-and-mcus/lpc-cortex-m-mcus/lpc800-series-cortex-m0-plus-mcus/low-cost-microcontrollers-mcus-based-on-arm-cortex-m0-plus-cores:LPC81X_LPC83X), a no-frills Arm Cortex-M0+ microcontroller with 8KB flash and 2KB RAM. Despite its limited peripheral set, (it doesn't even have an ADC) it gets the job done. I chose this chip for 3 reasons:

1. It has a serial bootloader in ROM and can be programmed with just an FTDI cable
2. The peripherals are simple enough that vendor SDK code isn't required
3. I had a couple of them lying around


The real-time clock is the [NXP PCF2129](https://www.nxp.com/products/analog/signal-chain/real-time-clocks/rtcs-with-temperature-compensation/accurate-rtc-with-integrated-quartz-crystal-for-industrial-applications:PCF2129). It has a built-in temperature compensated oscillator with an advertised accuracy of ±3 ppm. Its feature set is similar to that of Maxim's high accurary real-time clocks like the [DS3234](https://www.mouser.com/ProductDetail/Maxim-Integrated/DS3234S?qs=sGAEpiMZZMuuBt6TL7D%2f6Oew70i91ENb), but it's a [third](https://www.mouser.com/ProductDetail/NXP-Semiconductors/PCF2129AT-2518?qs=sGAEpiMZZMvocn3OtlEZq6VFh71YhKpQ) of the price.

### Code notes

The driver and utility functions are written in C and the application code is C++11. 

The display state is encoded in a 64-bit integer. Since the display only has 60 segments (each digit has 10 segments; nine plus decimal point), 4 bits are unused. The mapping from bits to segments is irregular; the bit assignment was chosen to simplify the circuit layout. Each 32-bit word holds the data for 30 segments (3 digits) with 2 bits unused.

For ease of programming, application code treats the display as six 16-bit halfwords, one per digit, with the lower 10 bits mapping to segments `a` through `i` plus decimal point. The function `three_digits_packed()` converts three 16-bit values to the particular 32-bit internal representation. And it's written in assembly because I couldn't help myself. :)

SysTick is the only interrupt used. It runs at a rate of 480 Hz. On each iteration, the segment pattern is shifted out to the drivers. This handles the segment preheating and PWM dimming. 480 Hz allows for eight levels of brightness without visible flicker.

One oddity is that the display shift registers are always cleared (by shifting out 64 zeros) before shifting out the segment pattern. I was encountering a weird issue where the leftmost 3 digits would glitch out occasionally. I assumed this was due to a timing glitch, but rather than tracking down the root cause, I found that the glitch disappeared if the shift registers were zeroed first. So I hacked that up and moved on. :)

Every 8 ticks (60 Hz) the button state is read and the `/RTC_INT` line is checked. The PCF2129AT is configured to pulse `/RTC_INT` low once per second. The pin change interrupt is used to detect falling edges, but to keep the code simple and deterministic, I poll the interrupt flag rather than use the actual asynchronous interrupt.

At any time, only one "app" has control of the display. An "app" can be thought of as a state in a state machine. Each app exposes two public methods. `init()` is called when an app becomes active. `update()` is called at 60 Hz with a 32-bit bitmask of asserted input events. It returns a pointer to another app, or `NULL` if no state change is required. The methods `timeout()` and `flash_mask()` are used by the main loop to handle common logic for apps that "time out" after a period of no input and apps that want to flash portions of the display.

As much as possible, all date arithmetic is performed in binary coded decimal.

The PCF2129 does not have dedicated battery-backed SRAM for storing settings. However, since the clock doesn't need the day and weekday alarms, I can use them as 10 bits of general-purpose battery-backed storage, saving me from using a more expensive RTC or using the battery to keep the microcontroller powered while unplugged.

The SCTimer peripheral is used to generate two complementary square waves which drive the piezo beeper.

### Special thanks

Kevin Townsend for his [LPC810 base code](https://github.com/microbuilder/LPC810_CodeBase).

James Coxon for the only [reasonable SCTimer PWM example](https://github.com/jamescoxon/LPC812/blob/master/src/pwm.c) I could find online.
