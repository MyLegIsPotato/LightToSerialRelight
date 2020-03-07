# LightToSerialRelight
A Beat Saber mod that allows you to control your Arduino LED Strip based on Beat Saber events.

This mod is **updated version** of the mod created by [D3rPole](https://github.com/D3rPole/BS-LightToSerial).
It allows you to communicate with Arduino based on events to display preset animations and patterns on your wb2812 adressable LED strip. The mod automatically gets custom colors from Beat Saber and makes them fully saturated before reaching arduino, it may be the reason if some colors on LED strip get displayed incorrectly. Most of the times it works as intended.

[Gameplay](https://www.youtube.com/watch?v=9OCgU-t3w0Y)

## Mod installation
Just download the .dll file and drop it into your BeatSaber folder/Plugins.

You will also need other mods, that is [BeatSaberMarkupLanguage](https://github.com/monkeymanboy/BeatSaberMarkupLanguage) and [BeatSaberUtils](https://github.com/Kylemc1413/Beat-Saber-Utils).

## Setup

Before you play Beat Saber you will need to download arduino program and upload it to your board. Set all the settings in the top part to match your setup, that includes:
```
#define LED_PIN     9
#define NUM_LEDS    120
```
and probably:
```
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
```
Can't tell if this program works with other types of LEDs.

You will also need to install FastLED library. To do so go to your Arduino IDE -> Tools -> Manage Libraries -> Search for: *FastLED* -> Select version **3.3.0** -> Click install

Default baudrate (1200) was tested to work fine and there is no need to use any faster ones. You can do it anyway, there is this functionality. After selecting your port in game setting (you can find which one in Arduino IDE) and clicking **apply** the LED strip should turn on to white-ish palette with colored ends and with a breathing animation, it's a standby signal telling you that connection was successful.

## Other information
Remember to report any bugs. If you have any question regarding modding don't hestitate to ask, it wasn't easy at the start for me too.

Special thanks to:
- [D3rPole](https://github.com/D3rPole), for idea and old code that got basically rewritten from beginning.
- [Shadnix-was-taken](https://github.com/Shadnix-was-taken) whos code was used for my learning :) 
- Guys at Beat Saber Modding Group Discord. They are the best <3
