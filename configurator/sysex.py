import rtmidi
import time

# Sysex
SYSTEM_EXCLUSIVE        = 0xF0
END_OF_EXCLUSIVE        = 0xF7

MANUFACTURER_ID         = 0x60
DEVICE_ID               = 0x01
COMMAND_ALIVE           = 0x41
COMMAND_READ_VERSION    = 0x51
COMMAND_READ_SETTINGS   = 0x52
COMMAND_READ_PRESET     = 0x53
COMMAND_WRITE_SETTINGS  = 0x61
COMMAND_WRITE_PRESET    = 0x62
RESPONSE_SUCCESS        = 0x71
RESPONSE_ERROR          = 0x72
TIMEOUT                 = 2.0

class ColorSpraySysex:
    def __init__(self):
        self.midiout = rtmidi.MidiOut()
        self.midiin = rtmidi.MidiIn()

    def get_port_count(self):
        return self.midiout.get_port_count()
    def get_port_name(self, i):
        return self.midiout.get_port_name(i)

    def is_connected(self):
        if not self.midiout.is_port_open() or not self.midiin.is_port_open():
            return False
        return True

    def connect(self, portnum):
        if portnum < 0 or portnum >= self.midiout.get_port_count():
            return False

        if self.midiout.is_port_open():
            self.midiout.close_port()

        if self.midiin.is_port_open():
            self.midiin.close_port()

        self.midiout.open_port(portnum)
        self.midiin.open_port(portnum)

        # Prevent ignoring SysEx
        self.midiin.ignore_types(False)

        return self.is_connected()

    def disconnect(self):
        if not self.midiout.is_port_open() and not self.midiin.is_port_open():
            return False

        if not self.midiout.is_port_open():
            self.midiout.close_port()
        if not self.midiin.is_port_open():
            self.midiin.close_port()

        return True

    def send_command(self, command):
        if type(command) != list:
            command = [command]
        data = bytes([SYSTEM_EXCLUSIVE, MANUFACTURER_ID, DEVICE_ID] + command + [END_OF_EXCLUSIVE])
        self.midiout.send_message(data)
    def send_data(self, command, data):
        if type(command) != list:
            command = [command]
        if type(data) != list:
            data = [data]
        self.send_command(command + data)

    def get_response(self):
        if not self.midiin.is_port_open():
            return False
        start = time.time()
        while True:
            # Python-RtMidi
            msg = self.midiin.get_message()
            if msg:
                message, deltatime = msg
                if len(message) >= 4 and message[0] == SYSTEM_EXCLUSIVE and message[1] == MANUFACTURER_ID and message[2] == DEVICE_ID:
                    data = []
                    for i in range(1, len(message)):
                        if message[i] == END_OF_EXCLUSIVE:
                            return data
                        data.append(message[i])

            # PyRtMidi
            #msg = self.midiin.getMessage(TIMEOUT / 16)
            #if msg and msg.isSysEx():
            #    data = list(msg.getSysExData())
            #    if data[0] == MANUFACTURER_ID and data[1] == DEVICE_ID:
            #        return data
            if time.time() - start > TIMEOUT:
                break
        return False

    def check_alive(self):
        self.send_command(COMMAND_ALIVE)
        data = self.get_response()
        return data and len(data) == 3 and data[2] == RESPONSE_SUCCESS
    def check_version(self, version):
        self.send_command(COMMAND_READ_VERSION)
        data = self.get_response()
        return data and len(data) == 4 and data[2] == RESPONSE_SUCCESS and data[3] == version

    def read_settings(self, version):
        self.send_command(COMMAND_READ_SETTINGS)
        data = self.get_response()
        if not (data and len(data) > 4 and data[2] == RESPONSE_SUCCESS and data[3] == version):
            return False
        return data
    def read_preset(self, version, i):
        self.send_command([COMMAND_READ_PRESET, i])
        data = self.get_response()
        if not (data and len(data) > 4 and data[2] == RESPONSE_SUCCESS and data[3] == version):
            return False
        return data

    def write_settings(self, version, data):
        self.send_data([COMMAND_WRITE_SETTINGS, version], data)
        data = self.get_response()
        return data and len(data) == 3 and data[2] == RESPONSE_SUCCESS
    def write_preset(self, version, i, data):
        self.send_data([COMMAND_WRITE_PRESET, version, i], data)
        data = self.get_response()
        return data and len(data) == 3 and data[2] == RESPONSE_SUCCESS
