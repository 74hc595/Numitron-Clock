# IV-19 Numitron Alarm Clock

This is a digital alarm clock that uses Soviet surplus IV-19 (ИВ-19) incandescent ["numitron"](http://www.decadecounter.com/vta/tubepage.php?item=10) displays. 


**[Video overview](https://www.youtube.com/watch?v=N-rBARzu5Fk)**


## Features

The feature set is similar to that of my [Panaplex clock](https://github.com/74hc595/Panaplex-Clock):

- Six digit display (hours/minutes/seconds and month/day/year)
- Powered from a 5V USB charger
- Coin cell battery backup keeps time when unplugged
- Highly accurate oscillator; keeps time within ±3 parts per million
- Piezo beeper alarm
- Automatic leap year correction up to 2099
- Automatic daylight saving time correction (US) up to 2099
- 4-level brightness control
- Sleep mode: can be configured to keep the display blanked and only show the time for a few seconds at the press of a button


## Background

Unlike Nixie tubes, a numitron is not filled with gas and doesn't require high voltage; each segment is literally just an incandescent filament, same as you'd find in a light bulb.

They were a technological stopgap between nixie tubes and LEDs. They also seem to have been used extensively in avionics equipment. (and in the climax of the movie _WarGames_!) I remember seeing them on fuel pumps in the late 1980s.

I found these particular displays on eBay and despite their insane price (US$25 each!) I bought them because of their very uncommon [nine-segment layout](https://en.wikipedia.org/wiki/Nine-segment_display). I've only found evidence of two other people using these displays for projects—and I had to resort to a [Yandex image search](https://yandex.ru/images/search?text=ИВ-20)!

I was able to find a scan of the original Russian datasheet on [Dieter's tube data site](http://www.tube-tester.com/sites/nixie/dat_arch.htm). I've included it in this repo, along with a rough Google translation.


## Hardware

The design is fairly straightforward. Numitrons don't multiplex well, so all 60 segments are direct-driven. They don't require high voltage (I'm driving them with a 3.7V supply) so I used four [STP16CP05](https://www.mouser.com/ProductDetail/STMicroelectronics/STP16CP05MTR/?qs=sGAEpiMZZMvsUbCbgzcXuExUir1Co9YI) constant-current shift register drivers. The segments draw about 22 mA _each_—over an order of magnitude more than LED displays. Multiply that by 60, then multiply that by 3.7, and you'll find that the display draws 4.8 watts with all segments illuminated!

For the brains, I use an [NXP LPC811](https://www.nxp.com/products/processors-and-microcontrollers/arm-based-processors-and-mcus/lpc-cortex-m-mcus/lpc800-series-cortex-m0-plus-mcus/low-cost-microcontrollers-mcus-based-on-arm-cortex-m0-plus-cores:LPC81X_LPC83X)—a relatively bare-bones Arm Cortex-M0+ microcontroller.

Like the Panaplex clock, the enclosure design is an homage to the wood-grain Heathkit clocks from the 1970s, like the [GC-1005](http://tubetime.us/index.php/2012/04/). (Hi Eric!) It's made of 1/8" laser-cut walnut hardwood. The faceplate is transparent 1/8" acrylic, also laser-cut. Unlike neon/Nixie tubes, which emit a very specific wavelength of orange light, the numitron filaments give off a wide spectrum of light, so you can use any color filter you want! I made transparent red, green, and blue faceplates. The blue is best in my opinion—the digits are a deep turquoise color that I haven't been able to photograph successfully.


## Operation

There is a column of three pushbuttons on the back. From top to bottom, they are labeled `+` (Plus), `◉` (Enter), and `−` (Minus).


#### Time display

At the 26th second of even minutes, the date is shown (month/day/year, sorry rest-of-the-world) for 5 seconds. If the backup battery is failing or not present, the display will also briefly show `BAT LO`.

During time display, each button has two functions:

- **Press** `+`: change brightness
- **Hold** `+`: enter sleep mode
- **Press** `◉`: set alarm (see below)
- **Hold** `◉`: set time (see below)
- **Press** `−`: show date for 5 seconds
- **Hold** `−`: enter options menu (see below)


### Setting the time

The hours should be flashing. Use `+`/`−` to change the hour. Press `◉` to advance to the minutes and use `+`/`−` to change the minute. Pressing `◉` finalizes the time, resetting the seconds to :00 and returning you to the normal time display.

Hold `◉` to cancel and return to the normal time display. Any changes to the time are discarded.


### Setting the alarm

The letter `A.` should be flashing. Pressing `+` or `−` toggles the alarm on and off. When the alarm is active, the decimal point of the rightmost digit will illuminate.

Press `◉` to advance to the hours. Use `+`/`−` to change the hour. Press `◉` to advance to the minutes and use `+`/`−` to change the minute.

Hold `◉` to cancel and return to the normal time display. Any changes to the alarm time are discarded.


### Silencing the alarm

When the alarm is going off, hold any button to silence it.


### Alarms and loss of power

The alarm time and state are saved in battery-backed memory. If power is lost, but restored before the alarm's set time, the alarm will go off normally. If power is restored _after_ the alarm's set time, the alarm will _not_ go off.


### Options menu

Pressing `+` and `−` cycle through the menu options:

- `zzz` - turn sleep mode on/off (`Y`/`N`)
- `DATE>` - press `◉` to set date (see below)
- `A.DST` - turn automatic daylight saving time correction on/off (`Y`/`N`)
- `FONT` - switch between 9-segment font (`1`) and 7-segment font (`2`) for time/date display
- `CYCL` - turn date cycling on even minutes on/off (`Y`/`N`)
- `STYLE>` - press `◉` to change time display style (see below)
- Backup battery status: will show `BAT OK` or `BAT LO`
- Firmware version, e.g. `Ver 1.2`
- `DONE>` - press `◉` to return to normal time display

Hold `◉` to cancel and return to the normal time display.


### Setting the date

The month should be flashing. Use `+`/`−` to change the month. Press `◉` to advance to the date and use `+`/`−` to change the date. Press `◉` to advance to the year and use `+`/`−` to change the year. Press `◉` to finalize the date.

Hold `◉` to cancel and return to the menu. Any changes to the date are discarded.


### Setting the display style

Press `+` and `−` to cycle through four time display options:

- `hh.mm.ss` (hour, minute, second)
- `hh.mm` (hour and minute only)
- `h.mm a` (12-hour display with hour, minute, and AM/PM indicator. `◿`=AM, `◸`=PM)
- `hh . mm` (hour and minute only)

Press `◉` to return to the menu.


### Flashing new firmware

Cycle through menu options until the firmware version is displayed. Hold `+` to enter the bootloader. The display will show `---`. You can now connect a 5V FTDI cable and use the `lpc21isp` utility to flash a new firmware image.

To return to normal operation, disconnect and reconnect the power.


## Power considerations

The clock can draw up to 1.5A during operation. It's recommended that you use a high-quality USB power adapter (10 watts or higher), and a "28/24 AWG" USB mini-B cable.

## Version history

### Version 1.2 (March 21, 2019)

- Reduced number of brightness levels to 4.
- Added option to change the number font to one that uses a "standard" 7-segment layout which does not use the diagonal segments. Some may find this more readable.

### Version 1.1 (April 29, 2018)

- 12-hour display style added. Selectable from the `STYLE>` menu. When enabled, 12-hour format with AM/PM indicator (`◿`=AM, `◸`=PM) is used when displaying the time, setting the time, and setting the alarm.

### Version 1.0 (February 11, 2018)

- Initial release
