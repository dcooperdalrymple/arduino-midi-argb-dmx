import tkinter
from tkinter import *
from tkinter import messagebox
from tkinter import ttk
from tkinter import filedialog
from ScrollableNotebook import *
import time
from enum import Enum
import math
import colorsys
import json
import os

from sysex import ColorSpraySysex, MANUFACTURER_ID, DEVICE_ID

SETTINGS_VERSION        = 2
PRESET_COUNT            = 15

root = Tk()
root.title('Color Spray Configurator')
if os.name == 'nt':
    root.iconbitmap(bitmap="{}/icon.ico".format(os.path.dirname(__file__)))
elif os.name == 'posix':
    root.iconbitmap(bitmap="@{}/icon.xbm".format(os.path.dirname(__file__)))
root.geometry("640x360")
root.minsize(360, 360)

sysex = ColorSpraySysex()

midi_portnum = IntVar(root, -1, name = "Midi Port")
def sysex_connect():
    if midi_portnum.get() < 0 or midi_portnum.get() >= sysex.get_port_count():
        midi_portnum.set(-1)
        progresstext.configure(text="Invalid port selected for midi connection.")
        messagebox.showerror("Unable to Connect", "Invalid port number.")
        return False
    if not sysex.connect(midi_portnum.get()):
        midi_portnum.set(-1)
        progresstext.configure(text="Unable to connect to midi port, \"{}\".".format(sysex.get_port_name(midi_portnum.get())))
        messagebox.showerror("Unable to Connect", "Unable to connect to midi port for unknown reason.")
        return False
    progresstext.configure(text="Successfully connected to midi port, \"{}\".".format(sysex.get_port_name(midi_portnum.get())))
    return True
def sysex_disconnect():
    if not sysex.is_connected():
        midi_portnum.set(-1)
        return False

    sysex.disconnect()
    if midi_portnum.get() >= 0:
        progresstext.configure(text="Disconnected from midi port, {}.".format(sysex.get_port_name(midi_portnum.get())))
    midi_portnum.set(-1)
    return True

class ColorMode(Enum):
    ColorOff    = 0
    SolidRgb    = 1
    SolidHsv    = 2
    Palette     = 3

class ArgbType(Enum):
    NONE        = 0
    NEOPIXEL    = 1
#    SM16703     = 2
#    TM1829      = 3
#    TM1812      = 4
    TM1809      = 5
    TM1804      = 6
    TM1803      = 7
    UCS1903     = 8
#    UCS1903B    = 9
#    UCS1904     = 10
#    UCS2903     = 11
    WS2812      = 12
#    WS2852      = 13
    WS2812B     = 14
#    GS1903      = 15
#    SK6812      = 16
#    SK6822      = 17
#    APA106      = 18
#    PL9823      = 19
    WS2811      = 20
    WS2813      = 21
#    APA104      = 22
#    WS2811_400  = 23
#    GE8822      = 24
#    GW6205      = 25
#    GW6205_400  = 26
#    LPD1886     = 27
#    LPD1886_8BIT = 28

class Palette(Enum):
    Cloud           = 0
    Lava            = 1
    Ocean           = 2
    Forest          = 3
    Rainbow         = 4
    Rainbow_Stripe  = 5
    Party           = 6
    Heat            = 7

class ColorRGB:
    def __init__(self, r=0, g=0, b=0):
        self.r = 0
        self.g = 0
        self.b = 0
    def getData(self):
        return {
            'r': self.r,
            'g': self.g,
            'b': self.b
        }
    def setData(self, data):
        if 'r' in data:
            self.r = int(data['r'])
        if 'g' in data:
            self.g = int(data['g'])
        if 'b' in data:
            self.b = int(data['b'])
        return True
class ColorHSV:
    def __init__(self, h=0, s=0, v=0):
        self.h = 0
        self.s = 0
        self.v = 0
    def getData(self):
        return {
            'h': self.h,
            's': self.s,
            'v': self.v
        }
    def setData(self, data):
        if 'h' in data:
            self.h = int(data['h'])
        if 's' in data:
            self.s = int(data['s'])
        if 'v' in data:
            self.v = int(data['v'])
        return True

def build_combobox(parent, text, var, values, default, callback):
    frame = ttk.Frame(parent, borderwidth=1, padding=5)
    frame.pack(fill="x")
    label = ttk.Label(frame, text=text)
    label.pack(fill="x")
    combo = ttk.Combobox(frame, textvariable=var)
    combo['values'] = values
    combo.bind('<<ComboboxSelected>>', callback)
    combo.pack(fill="x")
    var.set(default)
    return frame, combo

def build_checkbox(parent, text, var, default, callback, title="", description=""):
    frame = ttk.Frame(parent, borderwidth=1, padding=5)
    frame.pack(fill="x")
    if title:
        label = ttk.Label(frame, text=title)
        label.pack(fill="x")
    button = ttk.Checkbutton(frame, text=text, command=callback, variable=var, onvalue=True, offvalue=False)
    button.pack(fill="x", expand=True)
    if description:
        label = ttk.Label(frame, text=title)
        label.pack(fill="x")
    var.set(default)
    return frame, button

def build_slider(parent, text, var, from_, to, default, callback):
    frame = ttk.Frame(parent, borderwidth=1, padding=5)
    frame.pack(fill="x")
    label = ttk.Label(frame, text=text)
    label.pack(fill="x")
    slider = ttk.Scale(frame, from_=from_, to=to, orient="horizontal", variable=var, command=callback)
    slider.pack(fill="x")
    var.set(default)
    return frame, slider

def build_number(parent, text, var, from_, to, default, callback):
    frame = ttk.Frame(parent, borderwidth=1, padding=5)
    frame.pack(fill="x")
    label = ttk.Label(frame, text=text)
    label.pack(fill="x")
    number = ttk.Spinbox(frame, from_=from_, to=to, textvariable=var, command=callback, wrap=False, increment=1.0)
    number.pack(fill="x")
    var.set(default)
    return frame, number

def build_color_slider(parent, text, var, callback, default=0):
    frame = ttk.Frame(parent, borderwidth=1, padding=5)
    frame.pack(fill="x")
    label = ttk.Label(frame, text=text, padding=(5, 0, 5, 0))
    label.pack(side="left")
    slider_frame = ttk.Frame(frame, borderwidth=0, padding=(5, 0, 5, 0))
    slider_frame.pack(side="left", fill="x", expand=True)
    slider = ttk.Scale(slider_frame, from_=0, to=127, orient="horizontal", variable=var, command=callback)
    slider.pack(expand=True, fill="x")
    number = ttk.Spinbox(frame, from_=0, to=127, textvariable=var, command=callback, wrap=False)
    number.pack(side="right")
    var.set(default)
    return frame, slider, number

class SettingsData:
    data_keys = ["midiChannel", "midiThru", "brightness", "argbCount", "argbType", "dmxCount", "dmxChannelSize", "dmxChannelOffset", "dmxBrightness", "dmxBrightnessChannel", "preset"]
    def __init__(self):
        self.midiChannel = 0
        self.midiThru = True
        self.brightness = 64
        self.argbCount = 12
        self.argbType = ArgbType.WS2812B
        self.dmxCount = 1
        self.dmxChannelSize = 3
        self.dmxChannelOffset = 1
        self.dmxBrightness = 127
        self.dmxBrightnessChannel = 0
        self.preset = 0
    def getData(self):
        return {
            'midiChannel': self.midiChannel,
            'midiThru': self.midiThru,
            'brightness': self.brightness,
            'argbCount': self.argbCount,
            'argbType': self.argbType.value,
            'dmxCount': self.dmxCount,
            'dmxChannelSize': self.dmxChannelSize,
            'dmxChannelOffset': self.dmxChannelOffset,
            'dmxBrightness': self.dmxBrightness,
            'dmxBrightnessChannel': self.dmxBrightnessChannel,
            'preset': self.preset
        }
    def setData(self, data):
        if 'midiChannel' in data:
            self.midiChannel = int(data['midiChannel'])
        if 'midiThru' in data:
            self.midiThru = bool(data['midiThru'])
        if 'brightness' in data:
            self.brightness = int(data['brightness'])
        if 'argbCount' in data:
            self.argbCount = int(data['argbCount'])
        if 'argbType' in data:
            self.argbType = ArgbType(int(data['argbType']))
        if 'dmxCount' in data:
            self.dmxCount = int(data['dmxCount'])
        if 'dmxChannelSize' in data:
            self.dmxChannelSize = int(data['dmxChannelSize'])
        if 'dmxChannelOffset' in data:
            self.dmxChannelOffset = int(data['dmxChannelOffset'])
        if 'dmxBrightness' in data:
            self.dmxBrightness = int(data['dmxBrightness'])
        if 'dmxBrightnessChannel' in data:
            self.dmxBrightnessChannel = int(data['dmxBrightnessChannel'])
        if 'preset' in data:
            self.preset = int(data['preset'])
        self.updateVars()
        return True
    def getFrame(self, parent):
        if hasattr(self, "frame") and self.frame:
            return self.frame

        self.frame = ttk.Frame(parent, width=320, height=240)
        self.frame.pack(fill="both", expand=True)

        midi_frame = ttk.Frame(self.frame)
        midi_frame.pack(fill="x")

        self.midiChannel_var = StringVar()
        channel_frame, channel_combo = build_combobox(midi_frame, "Midi Channel", self.midiChannel_var, ["Omni"] + [str(i) for i in range(1, 17)], "Omni", self.updateMidiChannel)
        channel_frame.pack(side="left", expand=True, fill="x")

        self.midiThru_var = BooleanVar()
        thru_frame, thru_check = build_checkbox(midi_frame, "Midi Thru", self.midiThru_var, self.midiThru, self.updateMidiThru)
        thru_frame.pack(side="bottom")

        self.brightness_var = IntVar()
        build_slider(self.frame, "Global Brightness", self.brightness_var, 0, 127, self.brightness, self.updateBrightness)

        argb_frame = ttk.Frame(self.frame, borderwidth=0, padding=0)
        argb_frame.pack(fill="x")

        self.argbCount_var = StringVar()
        argb_count_frame, argb_count_number = build_number(argb_frame, "ARGB Count", self.argbCount_var, 1, 127, str(self.argbCount), self.updateArgbCount)
        argb_count_frame.pack(side="left", fill="x", expand=True)

        self.argbType_var = StringVar()
        argb_type_frame, argb_type_combo = build_combobox(argb_frame, "ARGB Type", self.argbType_var, [name for name, value in vars(ArgbType).items() if not name.startswith("_")], self.argbType.name, self.updateArgbType)
        argb_type_frame.pack(side="right", fill="x", expand=True)

        dmx_channel_row = ttk.Frame(self.frame, borderwidth=0, padding=0)
        dmx_channel_row.pack(fill="x")

        self.dmxCount_var = StringVar()
        dmx_count_frame, dmx_count_number = build_number(dmx_channel_row, "DMX Count", self.dmxCount_var, 1, 127, str(self.dmxCount), self.updateDmxCount)
        dmx_count_frame.pack(side="left", fill="x", expand=True)

        self.dmxChannelSize_var = StringVar()
        dmx_size_frame, dmx_size_number = build_number(dmx_channel_row, "DMX Channel Size", self.dmxChannelSize_var, 1, 32, str(self.dmxChannelSize), self.updateDmxChannelSize)
        dmx_size_frame.pack(side="left", fill="x", expand=True)

        self.dmxChannelOffset_var = StringVar()
        dmx_offset_frame, dmx_offset_number = build_number(dmx_channel_row, "DMX Channel RGB Offset", self.dmxChannelOffset_var, 1, 32, str(self.dmxChannelOffset), self.updateDmxChannelOffset)
        dmx_offset_frame.pack(side="left", fill="x", expand=True)

        dmx_brightness_row = ttk.Frame(self.frame, borderwidth=0, padding=0)
        dmx_brightness_row.pack(fill="x")

        self.dmxBrightness_var = IntVar()
        dmx_brightness_frame, dmx_brightness_slider = build_slider(dmx_brightness_row, "DMX Brightness", self.dmxBrightness_var, 0, 127, self.dmxBrightness, self.updateDmxBrightness)
        dmx_brightness_frame.pack(side="left", fill="x", expand=True)

        self.dmxBrightnessChannel_var = StringVar()
        dmx_brightness_channel_frame, dmx_brightness_channel_number = build_number(dmx_brightness_row, "DMX Brightess Channel Offset", self.dmxBrightnessChannel_var, 1, 32, str(self.dmxBrightnessChannel), self.updateDmxBrightnessChannel)
        dmx_brightness_channel_frame.pack(side="right", fill="x", expand=True)

        self.preset_var = StringVar()
        build_combobox(self.frame, "Default Preset", self.preset_var, ["None"] + ["Preset #"+str(i+1) for i in range(0, PRESET_COUNT)], "None", self.updatePreset)

        return self.frame
    def updateMidiChannel(self, event):
        if self.midiChannel_var.get() == "Omni":
            self.midiChannel = 0
        else:
            self.midiChannel = int(self.midiChannel_var.get())
    def updateMidiThru(self):
        self.midiThru = self.midiThru_var.get()
    def updateBrightness(self, event=False):
        self.brightness = self.brightness_var.get()
    def updateArgbCount(self):
        self.argbCount = int(self.argbCount_var.get())
    def updateArgbType(self, event=False):
        for name, value in vars(ArgbType).items():
            if name == self.argbType_var.get():
                self.argbType = ArgbType(value)
                break
    def updateDmxCount(self):
        self.dmxCount = int(self.dmxCount_var.get())
    def updateDmxChannelSize(self):
        self.dmxChannelSize = int(self.dmxChannelSize_var.get())
    def updateDmxChannelOffset(self):
        self.dmxChannelOffset = int(self.dmxChannelOffset_var.get())
    def updateDmxBrightness(self, event=False):
        self.dmxBrightness = self.dmxBrightness_var.get()
    def updateDmxBrightnessChannel(self):
        self.dmxBrightnessChannel = int(self.dmxBrightnessChannel_var.get())
    def updatePreset(self, event):
        if self.preset_var.get() == "None":
            self.preset = 0
        else:
            self.preset = int(self.preset_var.get()[len("Preset #"):])
    def updateVars(self):
        if self.midiChannel == 0:
            self.midiChannel_var.set("Omni")
        else:
            self.midiChannel_var.set(str(self.midiChannel))
        self.midiThru_var.set(self.midiThru)
        self.brightness_var.set(self.brightness)
        self.argbCount_var.set(str(self.argbCount))
        self.argbType_var.set(self.argbType.name)
        self.dmxCount_var.set(str(self.dmxCount))
        self.dmxChannelSize_var.set(str(self.dmxChannelSize))
        self.dmxChannelOffset_var.set(str(self.dmxChannelOffset))
        self.dmxBrightness_var.set(self.dmxBrightness)
        self.dmxBrightnessChannel_var.set(self.dmxBrightnessChannel)
        if self.preset == 0:
            self.preset_var.set("None")
        else:
            self.preset_var.set("Preset #{}".format(self.preset))

class PresetData:
    data_keys = ["note", "mode", "palette", "palette_speed", "color_rgb_r", "color_rgb_g", "color_rgb_b", "color_hsv_h", "color_hsv_s", "color_hsv_v"]
    def __init__(self, index):
        self.index = index
        self.note = 0
        self.mode = ColorMode.ColorOff
        self.palette = Palette.Cloud
        self.palette_speed = 1.0
        self.color_rgb = ColorRGB(0, 0, 0)
        self.color_hsv = ColorHSV(0, 0, 0)
    def getData(self, flat=False):
        if not flat:
            return {
                'note': self.note,
                'mode': self.mode.value,
                'palette': self.palette.value,
                'palette_speed': int(self.palette_speed * 32.0),
                'color_rgb': self.color_rgb.getData(),
                'color_hsv': self.color_hsv.getData()
            }
        else:
            return {
                'note': self.note,
                'mode': self.mode.value,
                'palette': self.palette.value,
                'palette_speed': int(self.palette_speed * 32.0),
                'color_rgb_r': self.color_rgb.r,
                'color_rgb_g': self.color_rgb.g,
                'color_rgb_b': self.color_rgb.b,
                'color_hsv_h': self.color_hsv.h,
                'color_hsv_s': self.color_hsv.s,
                'color_hsv_v': self.color_hsv.v
            }

    def setData(self, data):
        if 'note' in data:
            self.note = int(data['note'])
        if 'mode' in data:
            self.mode = ColorMode(int(data['mode']))
        if 'palette' in data:
            self.palette = Palette(int(data['palette']))
        if 'palette_speed' in data:
            self.palette_speed = float(data['palette_speed']) / 32.0
        if 'color_rgb' in data:
            self.color_rgb.setData(data['color_rgb'])
        else:
            if 'color_rgb_r' in data:
                self.color_rgb.r = data['color_rgb_r']
            if 'color_rgb_g' in data:
                self.color_rgb.g = data['color_rgb_g']
            if 'color_rgb_b' in data:
                self.color_rgb.b = data['color_rgb_b']
        if 'color_hsv' in data:
            self.color_hsv.setData(data['color_hsv'])
        else:
            if 'color_hsv_h' in data:
                self.color_hsv.h = data['color_hsv_h']
            if 'color_hsv_s' in data:
                self.color_hsv.s = data['color_hsv_s']
            if 'color_hsv_v' in data:
                self.color_hsv.v = data['color_hsv_v']
        self.updateVars()
        self.updateVisibility()
        self.updateColorRGB()
        self.updateColorHSV()
        return True
    def getFrame(self, parent):
        if hasattr(self, "frame") and self.frame:
            return self.frame

        self.frame = ttk.Frame(parent, width=320, height=240)
        self.frame.pack(fill="both", expand=True)

        self.note_var = StringVar()
        build_combobox(self.frame, "Midi Note", self.note_var, ["None"] + [self.getNoteName(i) for i in range(21, 128)], "None", self.updateNote)

        self.mode_var = StringVar()
        build_combobox(self.frame, "Mode", self.mode_var, [name for name, value in vars(ColorMode).items() if not name.startswith("_")], self.mode.name, self.updateMode)

        self.palette_row = ttk.Frame(self.frame)
        self.palette_row.pack(fill="x")
        self.palette_var = StringVar()
        build_combobox(self.palette_row, "Palette", self.palette_var, [name.replace("_", " ") for name, value in vars(Palette).items() if not name.startswith("_")], self.palette.name, self.updatePalette)
        self.palette_speed_var = IntVar()
        build_slider(self.palette_row, "Speed", self.palette_speed_var, 0, 127, int(self.palette_speed * 32.0), self.updatePaletteSpeed)

        self.color_frame = ttk.Frame(self.frame, borderwidth=1, padding=5)
        self.color_frame.pack(fill="x")

        self.color_rgb_row = ttk.Frame(self.color_frame)
        self.color_rgb_row.pack(fill="x")
        self.color_rgb_preview = tkinter.Frame(self.color_rgb_row, borderwidth=1, width=64, height=64, bg="#000000")
        self.color_rgb_preview.pack(side="left")
        color_rgb_frame = ttk.Frame(self.color_rgb_row, borderwidth=1, padding=5)
        color_rgb_frame.pack(side="right", expand=True, fill="x")
        self.color_r_var = StringVar()
        build_color_slider(color_rgb_frame, "R", self.color_r_var, self.updateColorR)
        self.color_g_var = StringVar()
        build_color_slider(color_rgb_frame, "G", self.color_g_var, self.updateColorG)
        self.color_b_var = StringVar()
        build_color_slider(color_rgb_frame, "B", self.color_b_var, self.updateColorB)

        self.color_hsv_row = ttk.Frame(self.color_frame)
        self.color_hsv_row.pack(fill="x")
        self.color_hsv_preview = tkinter.Frame(self.color_hsv_row, borderwidth=1, width=64, height=64, bg="#000000")
        self.color_hsv_preview.pack(side="left")
        color_hsv_frame = ttk.Frame(self.color_hsv_row, borderwidth=1, padding=5)
        color_hsv_frame.pack(fill="x", side="right", expand=True)
        self.color_h_var = StringVar()
        build_color_slider(color_hsv_frame, "H", self.color_h_var, self.updateColorH)
        self.color_s_var = StringVar()
        build_color_slider(color_hsv_frame, "S", self.color_s_var, self.updateColorS)
        self.color_v_var = StringVar()
        build_color_slider(color_hsv_frame, "V", self.color_v_var, self.updateColorV)

        self.updateVisibility()
        return self.frame
    note_names = ["C{0}", "C#{0}/Db{0}", "D{0}", "D#{0}/Eb{0}", "E{0}", "F{0}", "F#{0}/Gb{0}", "G{0}", "G#{0}/Ab{0}", "A{0}", "A#{0}/Bb{0}", "B{0}"]
    def getNoteName(self, i):
        return "{} ({})".format(i, self.note_names[i % 12].format(math.floor(i / 12) - 1))
    def updateNote(self, event):
        if self.note_var.get() == "None":
            self.note = 0
        else:
            note_str = self.note_var.get()
            self.note = int(note_str[:note_str.find("(")])
    def updateMode(self, event):
        for name, value in vars(ColorMode).items():
            if name == self.mode_var.get():
                self.mode = ColorMode(value)
                break
        self.updateVisibility()
    def updatePalette(self, event):
        selected = self.palette_var.get().replace(" ", "_")
        for name, value in vars(Palette).items():
            if name == selected:
                self.palette = Palette(value)
                break
    def updatePaletteSpeed(self, event=False):
        self.palette_speed = self.palette_speed_var.get() / 32.0
    def updateColorR(self, event=False):
        self.color_r_var.set(str(int(float(self.color_r_var.get()))))
        self.color_rgb.r = int(math.floor(float(self.color_r_var.get())))
        self.updateColorRGB()
    def updateColorG(self, event=False):
        self.color_g_var.set(str(int(float(self.color_g_var.get()))))
        self.color_rgb.g = int(math.floor(float(self.color_g_var.get())))
        self.updateColorRGB()
    def updateColorB(self, event=False):
        self.color_b_var.set(str(int(float(self.color_b_var.get()))))
        self.color_rgb.b = int(math.floor(float(self.color_b_var.get())))
        self.updateColorRGB()
    def updateColorRGB(self):
        hex = "#{:02X}{:02X}{:02X}".format(self.color_rgb.r * 2, self.color_rgb.g * 2, self.color_rgb.b * 2)
        self.color_rgb_preview.configure(bg=hex)
    def updateColorH(self, event=False):
        self.color_h_var.set(str(int(float(self.color_h_var.get()))))
        self.color_hsv.h = int(math.floor(float(self.color_h_var.get())))
        self.updateColorHSV()
    def updateColorS(self, event=False):
        self.color_s_var.set(str(int(float(self.color_s_var.get()))))
        self.color_hsv.s = int(math.floor(float(self.color_s_var.get())))
        self.updateColorHSV()
    def updateColorV(self, event=False):
        self.color_v_var.set(str(int(float(self.color_v_var.get()))))
        self.color_hsv.v = int(math.floor(float(self.color_v_var.get())))
        self.updateColorHSV()
    def updateColorHSV(self):
        r,g,b = colorsys.hsv_to_rgb(self.color_hsv.h / 127.0, self.color_hsv.s / 127.0, self.color_hsv.v / 127.0)
        hex = "#{:02X}{:02X}{:02X}".format(int(r * 255), int(g * 255), int(b * 255))
        self.color_hsv_preview.configure(bg=hex)
    def updateVars(self):
        if self.note == 0:
            self.note_var.set("None")
        else:
            self.note_var.set(self.getNoteName(self.note))
        self.mode_var.set(self.mode.name)
        self.palette_var.set(self.palette.name)
        self.palette_speed_var.set(int(self.palette_speed * 32.0))
        self.color_r_var.set(str(int(float(self.color_rgb.r))))
        self.color_g_var.set(str(int(float(self.color_rgb.g))))
        self.color_b_var.set(str(int(float(self.color_rgb.b))))
        self.color_h_var.set(str(int(float(self.color_hsv.h))))
        self.color_s_var.set(str(int(float(self.color_hsv.s))))
        self.color_v_var.set(str(int(float(self.color_hsv.v))))
    def updateVisibility(self):
        self.palette_row.pack_forget()
        self.color_frame.pack_forget()
        self.color_rgb_row.pack_forget()
        self.color_hsv_row.pack_forget()
        if self.mode == ColorMode.SolidRgb:
            self.color_frame.pack(fill="x")
            self.color_rgb_row.pack(fill="x")
        elif self.mode == ColorMode.SolidHsv:
            self.color_frame.pack(fill="x")
            self.color_hsv_row.pack(fill="x")
        elif self.mode == ColorMode.Palette:
            self.palette_row.pack(fill="x")

class Settings:
    def __init__(self):
        self.data = SettingsData()
        self.presets = [PresetData(i) for i in range(0, PRESET_COUNT)]
    def getData(self):
        return {
            'manufacturer_id': MANUFACTURER_ID,
            'device_id': DEVICE_ID,
            'version': SETTINGS_VERSION,
            'settings': self.data.getData(),
            'presets': [self.presets[i].getData() for i in range(0, PRESET_COUNT)]
        }
    def setData(self, data):
        if not 'manufacturer_id' in data or data['manufacturer_id'] != MANUFACTURER_ID:
            return False
        if not 'device_id' in data or data['device_id'] != DEVICE_ID:
            return False
        if not 'version' in data or data['version'] != SETTINGS_VERSION:
            return False
        if not 'settings' in data or not 'presets' in data:
            return False
        self.data.setData(data['settings'])
        for i in range(0, min(PRESET_COUNT, len(data['presets']))):
            self.presets[i].setData(data['presets'][i])
        return True
settings = Settings()

def save():
    filename = filedialog.asksaveasfilename(title="Save JSON File", filetypes=[('json files', '*.json')])
    if filename:
        with open(filename, 'w') as f:
            json.dump(settings.getData(), f)
        return True
    return False

def load():
    filename = filedialog.askopenfilename(title="Open JSON File", filetypes=[('json files', '*.json')])
    if filename:
        with open(filename) as f:
            if not settings.setData(json.load(f)):
                messagebox.showerror("Unable to Load", "Invalid JSON settings file.")
                return False
            else:
                return True
    return False

def read_settings():
    data = sysex.read_settings(SETTINGS_VERSION)
    if not data:
        return False
    if len(data) != 4 + len(SettingsData.data_keys):
        return False
    settings_data = {}
    i = 0
    for key in SettingsData.data_keys:
        settings_data[key] = data[4+i]
        i += 1
    return settings_data

def read_preset(i):
    if i >= PRESET_COUNT:
        return False
    data = sysex.read_preset(SETTINGS_VERSION, i)
    if not data:
        return False
    if len(data) != 4 + len(PresetData.data_keys):
        return False
    preset_data = {}
    i = 0
    for key in PresetData.data_keys:
        preset_data[key] = data[4+i]
        i += 1
    return preset_data

def read():
    progressbar.start()

    progresstext.configure(text="Checking midi port connection.")
    if not sysex.is_connected():
        messagebox.showerror("Unable to Read", "Please connect to a midi port.")
        progresstext.configure(text="")
        progressbar.stop()
        return False

    progresstext.configure(text="Pinging device alive state.")
    if not sysex.check_alive():
        messagebox.showerror("Unable to Read", "Device not preset.")
        progresstext.configure(text="")
        progressbar.stop()
        return False

    progresstext.configure(text="Matching device settings version.")
    if not sysex.check_version(SETTINGS_VERSION):
        progressbar.stop()
        messagebox.showerror("Unable to Read", "Device firmware is incompatible.")
        progresstext.configure(text="")
        return False

    progresstext.configure(text="Reading global configuration from device.")
    settings_data = read_settings()
    if not settings_data:
        progressbar.stop()
        messagebox.showerror("Unable to Read", "Unknown error in settings response.")
        progresstext.configure(text="")
        return False
    settings.data.setData(settings_data)

    valid = True
    for i in range(0, PRESET_COUNT):
        progresstext.configure(text="Reading preset #{} from device.".format(i + 1))
        preset_data = read_preset(i)
        if not preset_data:
            valid = False
            break
        settings.presets[i].setData(preset_data)
    if not valid:
        progressbar.stop()
        messagebox.showerror("Unable to Read", "Unknown error in preset response.")
        progresstext.configure(text="")
        return False

    progressbar.stop()
    progresstext.configure(text="Full device read successful.")
    messagebox.showinfo("Read Successful", "Successfully read configuration and presets from device!")
    return True

def write():
    progressbar.start()

    progresstext.configure(text="Checking midi port connection.")
    if not sysex.is_connected():
        progressbar.stop()
        messagebox.showerror("Unable to Write", "Please connect to a midi port.")
        progresstext.configure(text="")
        return False

    progresstext.configure(text="Pinging device alive state.")
    if not sysex.check_alive():
        progressbar.stop()
        messagebox.showerror("Unable to Write", "Device not preset.")
        progresstext.configure(text="")
        return False

    progresstext.configure(text="Matching device settings version.")
    if not sysex.check_version(SETTINGS_VERSION):
        progressbar.stop()
        messagebox.showerror("Unable to Write", "Device firmware is incompatible.")
        progresstext.configure(text="")
        return False

    progresstext.configure(text="Writing global configuration to device.")
    if not sysex.write_settings(SETTINGS_VERSION, list(settings.data.getData().values())):
        progressbar.stop()
        messagebox.showerror("Unable to Write", "Unknown error in settings response.")
        progresstext.configure(text="")
        return False

    valid = True
    for i in range(0, PRESET_COUNT):
        progresstext.configure(text="Writing preset #{} to device memory.".format(i + 1))
        if not sysex.write_preset(SETTINGS_VERSION, i, list(settings.presets[i].getData(True).values())):
            valid = False
            break
    if not valid:
        progressbar.stop()
        messagebox.showerror("Unable to Write", "Unknown error in preset response.")
        progresstext.configure(text="")
        return False

    if not sysex.send_reset():
        progressbar.stop()
        progresstext.configure(text="Full device write successful; device not reset.")
        messagebox.showerror("Unable to Write", "Unable to reset device. You may need to reset the device manually for the new settings to take effect.")
        return False

    progressbar.stop()
    progresstext.configure(text="Full device write successful.")
    messagebox.showinfo("Write Successful", "Successfully wrote configuration and presets to device!")
    return True

def reset():
    progressbar.start()

    progresstext.configure(text="Checking midi port connection.")
    if not sysex.is_connected():
        progressbar.stop()
        messagebox.showerror("Unable to Reset", "Please connect to a midi port.")
        progresstext.configure(text="")
        return False

    progresstext.configure(text="Pinging device alive state.")
    if not sysex.check_alive():
        progressbar.stop()
        messagebox.showerror("Unable to Reset", "Device not preset.")
        progresstext.configure(text="")
        return False

    if not sysex.send_reset():
        progressbar.stop()
        progresstext.configure(text="Full device write successful; device not reset.")
        messagebox.showerror("Unable to Reset", "Unable to reset device. You may need to reset the device manually.")
        return False

    progressbar.stop()
    progresstext.configure(text="Device successfully reset.")
    return True

notebook = ScrollableNotebook(root, wheelscroll=True, tabmenu=True)

root.grid()
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)
root.rowconfigure(1, weight=0)
root.rowconfigure(2, weight=0)

frame = Frame(root)
frame.grid(column=0, row=0, sticky="news")
notebook.pack(fill="both", expand=True)

notebook.add(settings.data.getFrame(notebook), text="Config")
for i in range(0, PRESET_COUNT):
    notebook.add(settings.presets[i].getFrame(notebook), text=("Preset #"+str(i+1)))

progressframe = ttk.Frame(root, padding=(5, 0, 5, 0))
progressframe.grid(column=0, row=1, sticky="sew")

progresstext = ttk.Label(progressframe, justify=LEFT, text="")
progresstext.pack(fill="x")

progressbar = ttk.Progressbar(root, orient="horizontal", mode="indeterminate")
progressbar.grid(column=0, row=2, sticky="sew")
progressbar.stop()

mainmenu = Menu(frame)

filemenu = Menu(mainmenu, tearoff = 0)
filemenu.add_command(label = "Save", command = save)
filemenu.add_command(label = "Load", command = load)
filemenu.add_command(label = "Exit", command = root.destroy)
mainmenu.add_cascade(label = "File", menu = filemenu)

devicemenu = Menu(mainmenu, tearoff = 0)
portmenu = Menu(devicemenu, tearoff = 0)
def update_portmenu(event=False):
    if portmenu.index("end"):
        portmenu.delete(0, 'end')

    portmenu.add_command(label = "Refresh", command = refresh_portmenu)

    portmenu.add_radiobutton(variable = midi_portnum, command = sysex_disconnect, label = "None", value = -1)
    for i in range(0, sysex.get_port_count()):
        portmenu.add_radiobutton(variable = midi_portnum, command = sysex_connect, label = sysex.get_port_name(i), value = i)
def refresh_portmenu(event=False):
    sysex_disconnect()
    update_portmenu()
update_portmenu()
devicemenu.add_cascade(label = "Port", menu = portmenu)
devicemenu.add_command(label = "Read", command = read)
devicemenu.add_command(label = "Write", command = write)
devicemenu.add_command(label = "Reset", command = reset)
mainmenu.add_cascade(label = "Device", menu = devicemenu)

mainmenu.bind("<<MenuSelect>>", update_portmenu);

root.config(menu = mainmenu)
root.mainloop()
