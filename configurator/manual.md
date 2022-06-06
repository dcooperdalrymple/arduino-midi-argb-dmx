# Color Spray Configurator
> Configure the settings and presets of the Color Spray midi light controller.

## Table of Contents
1. [Config](#config)
   * [Midi Channel](#midi-channel)
   * [Midi Thru](#midi-thru)
   * [Global Brightness](#global-brightness)
   * [ARGB Count](#argb-count)
   * [ARGB Type](#argb-type)
   * [DMX](#dmx)
     * [Count](#dmx-count)
     * [Channel Size](#dmx-channel-size)
     * [RGB Offset](#dmx-rgb-offset)
     * [Brightness](#dmx-brightness)
     * [Brightness Channel](#dmx-brightness-channel)
2. [Presets](#preset)
   * [Program Number](#preset-program)
   * [Note](#preset-note)
   * [Mode](#preset-mode)
3. [Saving & Loading JSON Files](#json)
   * [Saving a JSON File](#json-save)
   * [Loading a JSON File](#json-load)
   * [Modifying JSON Files](#json-modify)
4. [Reading & Writing to Device](#device)
   * [Selecting the Device's Port](#device-port)
   * [Reading from the Device](#device-read)
   * [Writing from the Device](#device-write)
   * [Software Reset](#device-reset)
5. [Software Dependencies](#dependencies)

---

## Config<span id="config" />
The settings in this area will apply to all device operations. Most settings will take effect with a software reset after the device is written to, but some (such as the _ARGB Type_) may require a hardware reset before they take affect.

### Midi Channel<span id="midi-channel" />
Choose the desired midi channel to accept all midi messages: control change, program change, and note on. By default, the device is configured for _Omni_ mode where it will accept all messages. Changing the midi channel won't have any affect on reading and writing device configuration settings.

### Midi Thru<span id="midi-thru" />
If you'd like to transfer messages from the midi input to the output buffer, check the Midi Thru box. Messages will be transferred no matter what channel they are set to. The messages will be transferred according to the following table:

| Input Port | Output Ports      |
| ---------- | ----------------- |
| UART In    | UART Out, USB Out |
| USB In     | UART Out          |

> To prevent unnecessary loopback, USB is not set in Thru mode.
> By leaving Midi Thru enabled, it may slow down the device's responsiveness. If not needed, it is recommended that you leave Midi Thru disabled.

### Global Brightness<span id="global-brightness" />
By default, this setting is set to 50% where the color settings are unaffected. Drag the brightness slider down to lower the overall color intensity or higher to increase and potentially wash out your colors.

### ARGB Count<span id="argb-count" />
To individually control each LED in an ARGB strip, you will need to predefine the number of LEDs with this setting. You are allowed a maximum of 128 addressable LEDs and a minimum of 1.

### ARGB Type<span id="argb-type" />
WD2812B is set as the default ARGB controller, but depending on the type of LEDs you are controlling, you may need to change this setting. You can find more information on each chipset type [here](https://github.com/FastLED/FastLED/wiki/Overview#supported-chipsets). More chipset than the provided are supported but will require firmware re-compilation to enable.

### DMX<span id="dmx" />
You can control DMX lights simultaneously all on the same channels, but if you'd like to address each DMX light independently, you can use the following settings.

> It is not currently possible to configure lights independently. It is recommended that you use the same types of RGB lights together to avoid channel overlaps. Independent DMX device configuration may come in a future release.

DMX communication is limited to 512 channels. Make sure that your settings don't exceed 512 channels in total. The calculation to determine the total number of channels is as follows: `Count * Channel Size`.

#### Count<span id="dmx-count" />
Specify the number of devices that you'd like to control. You must at least have 1 device accounted for.

#### Channel Size<span id="dmx-channel-size" />
Specify the number of channels each device will be allocated. You may have to look up the specifications for the device's DMX utilization. A minimum of 3 channels can be allotted to account for the RGB channels and a maximum of 32 channels per device is allowed.

#### RGB Offset<span id="dmx-rgb-offset" />
Your device may have its RGB channels start after the first channel. Pick a value between 1 and 2 less that the total channel size to correct control your specific lights.

> By default, the order of color channels is *RGB*. If you need to change this, the firmware will need to be recompiled with a different setting for the `DMX_ORDER` definition.

#### Brightness<span id="dmx-brightness" />
Your DMX lights may have a specific channel used to control the overal brightness of the lights. You can specify the static brightness amount using this slider. By default, the brightness is set at its max of 127.

#### Brightness Channel<span id="dmx-brightness-channel" />
If your device has a dedicated brightness channel, you can specify it here as an offset of each device's first channel. If you leave this setting as 0, the brightness channel will be ignored.

---

## Presets<span id="preset" />
The Color Spray has 15 presets available. Each preset contains the same settings which can be configured live with midi control commands. These presets can be loaded when the device boots up, when a midi program change message is received, or when a midi note on message is received if the device is configured to do so.

### Program Number<span id="preset-program" />
If a midi program change message is received, the Color Spray will load up the appropriate preset from 1-15. If the program # is 0, the device will be set to the `ColorOff` mode. Program #'s from 16+ are reserved for changing color modes dynamically.

The assigned index of the preset will be given by the name of the selected preset tab.

### Note<span id="preset-note" />
By assigning a note value to a preset, the preset can be loaded by sending a midi note on message to the device that matches the assigned pitch. If multiple presets are assigned to the same pitch, the preset whose program number comes first will be loaded.

### Mode<span id="preset-mode" />
By default, each preset is set to the color mode `ColorOff`. This essentially deactivates the preset and prevents any light from emitting from connected devices. The available modes and their functionality are listed below:

#### SolidRGB<span id="preset-mode-solidrgb" />
Set a solid RGB color for all lights to be set. The Red, Green, and Blue settings can be set between 0 and 127 and be controlled using the slider from left to right or by manually typing in your desired value in the number box. Use the preview window to help you find the desired color.

#### SolidHSV<span id="preset-mode-solidhsv" />
Set a solid HSV color for all lights to be set. The Hue, Saturation, and Value settings can be set between 0 and 127 and be controlled using the slider from left to right or by manually typing in your desired value in the number box. Use the preview window to help you find the desired color.

#### Palette<span id="preset-palette" />
Loop through a pre-defined set of colors at a variable speed. There are 8 palette options available: Cloud, Lava, Ocean, Forest, Rainbow, Rainbow Stripe, Party, and Heat.

##### Speed<span id="preset-palette-speed" />
This value defines how fast the lights will rotate through the palette.

---

## Saving & Loading JSON Files<span id="json" />
Before writing to the device, it is recommended that you save a backup copy of your settings to a safe location in your drive.

### Saving a JSON File<span id="json-save" />
Under the *File* menu in the top left, there is a *Save* and *Load* option. By clicking the *Save* option, it will open up a file prompt for you to select your desired file name and location. After clicking the OK button, a copy of all Config and Presets settings will be saved in the JSON format.

### Loading a JSON File<span id="json-load" />
If you have a previously saved configurator backup file available, you can load it using the *Load* option under the *File* menu in the top left corner of the window. A file prompt should appear where you can select the `*.json` file which you saved earlier. If an incompatible file is loaded, the program will prompt you that an error occurred and you will need to try again. Backup files saved with previous versions of the configurator software may not be compatible with the current software. When the file has finished loading, all of the controls within the configurator should be updated to show the new settings. If a device is connected, it will not be automatically synchronized in this process.

### Modifying JSON Files<span id="json-modify" />
If it fits your workflow style, you can modify the Color Spray JSON files outside of the configurator using any plain text editor. As long as the Manufacturer ID, Device ID, and Version settings remain unchanged, the configurator should attempt to load the altered file. It may be possible to affect the program in unforeseen ways, so only attempt this if you are willing to lose your original data and potentially harm the device's firmware.

---

## Reading & Writing to Device<span id="device" />
Communication with the _Color Spray_ occurs over a standard MIDI connection using the Sysex protocol. The device will need to be connected in duplex through either a hardware MIDI interface or with the built-in USB-MIDI interface of the device. A USB connection is highly recommended during this process for it's ease of use and reliability.

### Selecting the Device's Port<span id="device-port" />
A list of all MIDI devices available to your computer will be listed within the *Device* menu under *Ports*. If you haven't yet connected your device, you can do so then select the top item under *Ports* labeled *Refresh*. This will disconnect any midi connection you may have activated and refresh the list of available MIDI ports.

The USB-MIDI interface of the device should be labeled as `Color Spray 32u4`. Select this port in the menu to initialize the connection. If there is any issue, you should be prompted of the error.

### Reading from the Device<span id="device-read" />
Reading all device settings from the _Color Spray_ is very simple. Once you've connected to its MIDI port, you'll need to selected *Read* under the *Device* menu. This will check the status and firmware of the device to ensure compatibility then begin to load the global configuration settings and all 15 presets from the device's memory.

If the read completed successfully, all settings within the configurator program will be updated to reflect the settings of the device. If you'd like to make a backup of the devices settings for future use, now would be a great time.

If any errors occurred during the process, you should be notified of the exact error which occurred. You may need to attempt the process again or manually reset and reconnect to the device if you continue to have problems.

### Writing from the Device<span id="device-write" />
Once you are confident with all of your settings and presets and have made any necessary backups, you can write these settings to the device's memory by selecting *Write* under the *Device* menu.

This process will work very similarly to the read process but send the data in the opposite direction. If a device isn't found, your device's firmware is incompatible, or there are any other unforeseen errors in the process, the operation will cancel and you will be prompted of the error.

After the global configuration and all 15 presets have been written to the device's memory, a reset command will be automatically sent to the device for it to use the new settings and reinitialize any connected addressable lighting devices.

### Software Reset<span id="device-reset" />
For any reason if you'd like to manually reset the device's software, you can do so using the *Reset* option under the *Device* menu. This will not perform a hardware reset to the device but instead reload any stored settings and reinitialize the device's configuration.

If you'd like to perform a hardware reset which may be necessary to resolve some issues, you can do so either by powering off and powering on the device or by pressing the internal reset button which may require disassembly.

---

## Software Dependencies<span id="dependencies" />
In order to run the configurator too, you will need Python 3.0+ and a few pip packages. After you've resolved all dependencies, run the program using `python configurator.py`.

### Linux (Ubuntu/Debian)<span id="dependencies-linux" />
```
sudo apt-get update
sudo apt-get install python3 python3-pip python3-is-python
pip install tkinter python-rtmidi tkinterweb
```

### Windows<span id="dependencies-windows" />
Download and install the latest version of Python 3.0+ [here](https://www.python.org/downloads/windows/). PIP should be installed by default since Python 3.4. Within the command line, run the following command to install all dependencies: `python -m pip install tkinter python-rtmidi tkinterweb markdown`.

### Packages<span id="dependencies-packages" />
* [pyython-rtmidi](https://github.com/SpotlightKid/python-rtmidi)
* [tkinterweb](https://github.com/Andereoo/TkinterWeb/)
* [ttkScrollableNotebook](https://github.com/muhammeteminturgut/ttkScrollableNotebook) _(included within repository)_
