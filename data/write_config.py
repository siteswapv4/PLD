import struct

gamepad_buttons = {
    "SOUTH": 0,
    "EAST": 1,
    "WEST": 2,
    "NORTH": 3,
    "BACK": 4,
    "GUIDE": 5,
    "START": 6,
    "LEFT_STICK": 7,
    "RIGHT_STICK": 8,
    "LEFT_SHOULDER": 9,
    "RIGHT_SHOULDER": 10,
    "DPAD_UP": 11,
    "DPAD_DOWN": 12,
    "DPAD_LEFT": 13,
    "DPAD_RIGHT": 14,
    "MISC1": 15,
    "RIGHT_PADDLE1": 16,
    "LEFT_PADDLE1": 17,
    "RIGHT_PADDLE2": 18,
    "LEFT_PADDLE2": 19,
    "TOUCHPAD": 20,
    "MISC2": 21,
    "MISC3": 22,
    "MISC4": 23,
    "MISC5": 24,
    "MISC6": 25,
    "LEFT_STICK_UP" : 26,
    "LEFT_STICK_DOWN" : 27,
    "LEFT_STICK_LEFT" : 28,
    "LEFT_STICK_RIGHT" : 29,
    "RIGHT_STICK_UP" : 30,
    "RIGHT_STICK_DOWN" : 31,
    "RIGHT_STICK_LEFT" : 32,
    "RIGHT_STICK_RIGHT" : 33,
    "LEFT_TRIGGER" : 34,
    "RIGHT_TRIGGER" : 35
}

scancodes = {
    "A": 4,
    "B": 5,
    "C": 6,
    "D": 7,
    "E": 8,
    "F": 9,
    "G": 10,
    "H": 11,
    "I": 12,
    "J": 13,
    "K": 14,
    "L": 15,
    "M": 16,
    "N": 17,
    "O": 18,
    "P": 19,
    "Q": 20,
    "R": 21,
    "S": 22,
    "T": 23,
    "U": 24,
    "V": 25,
    "W": 26,
    "X": 27,
    "Y": 28,
    "Z": 29,
    "1": 30,
    "2": 31,
    "3": 32,
    "4": 33,
    "5": 34,
    "6": 35,
    "7": 36,
    "8": 37,
    "9": 38,
    "0": 39,
    "RETURN": 40,
    "ESCAPE": 41,
    "BACKSPACE": 42,
    "TAB": 43,
    "SPACE": 44,
    "MINUS": 45,
    "EQUALS": 46,
    "LEFTBRACKET": 47,
    "RIGHTBRACKET": 48,
    "BACKSLASH": 49,
    "NONUSHASH": 50,
    "SEMICOLON": 51,
    "APOSTROPHE": 52,
    "GRAVE": 53,
    "COMMA": 54,
    "PERIOD": 55,
    "SLASH": 56,
    "CAPSLOCK": 57,
    "F1": 58,
    "F2": 59,
    "F3": 60,
    "F4": 61,
    "F5": 62,
    "F6": 63,
    "F7": 64,
    "F8": 65,
    "F9": 66,
    "F10": 67,
    "F11": 68,
    "F12": 69,
    "PRINTSCREEN": 70,
    "SCROLLLOCK": 71,
    "PAUSE": 72,
    "INSERT": 73,
    "HOME": 74,
    "PAGEUP": 75,
    "DELETE": 76,
    "END": 77,
    "PAGEDOWN": 78,
    "RIGHT": 79,
    "LEFT": 80,
    "DOWN": 81,
    "UP": 82,
    "NUMLOCKCLEAR": 83,
    "KP_DIVIDE": 84,
    "KP_MULTIPLY": 85,
    "KP_MINUS": 86,
    "KP_PLUS": 87,
    "KP_ENTER": 88,
    "KP_1": 89,
    "KP_2": 90,
    "KP_3": 91,
    "KP_4": 92,
    "KP_5": 93,
    "KP_6": 94,
    "KP_7": 95,
    "KP_8": 96,
    "KP_9": 97,
    "KP_0": 98,
    "KP_PERIOD": 99,
    "NONUSBACKSLASH": 100,
    "APPLICATION": 101,
    "POWER": 102,
    "KP_EQUALS": 103,
    "F13": 104,
    "F14": 105,
    "F15": 106,
    "F16": 107,
    "F17": 108,
    "F18": 109,
    "F19": 110,
    "F20": 111,
    "F21": 112,
    "F22": 113,
    "F23": 114,
    "F24": 115,
    "EXECUTE": 116,
    "HELP": 117,
    "MENU": 118,
    "SELECT": 119,
    "STOP": 120,
    "AGAIN": 121,
    "UNDO": 122,
    "CUT": 123,
    "COPY": 124,
    "PASTE": 125,
    "FIND": 126,
    "MUTE": 127,
    "VOLUMEUP": 128,
    "VOLUMEDOWN": 129,
    "KP_COMMA": 133,
    "KP_EQUALSAS400": 134,
    "INTERNATIONAL1": 135,
    "INTERNATIONAL2": 136,
    "INTERNATIONAL3": 137,
    "INTERNATIONAL4": 138,
    "INTERNATIONAL5": 139,
    "INTERNATIONAL6": 140,
    "INTERNATIONAL7": 141,
    "INTERNATIONAL8": 142,
    "INTERNATIONAL9": 143,
    "LANG1": 144,
    "LANG2": 145,
    "LANG3": 146,
    "LANG4": 147,
    "LANG5": 148,
    "LANG6": 149,
    "LANG7": 150,
    "LANG8": 151,
    "LANG9": 152,
    "ALTERASE": 153,
    "SYSREQ": 154,
    "CANCEL": 155,
    "CLEAR": 156,
    "PRIOR": 157,
    "RETURN2": 158,
    "SEPARATOR": 159,
    "OUT": 160,
    "OPER": 161,
    "CLEARAGAIN": 162,
    "CRSEL": 163,
    "EXSEL": 164,
    "KP_00": 176,
    "KP_000": 177,
    "THOUSANDSSEPARATOR": 178,
    "DECIMALSEPARATOR": 179,
    "CURRENCYUNIT": 180,
    "CURRENCYSUBUNIT": 181,
    "KP_LEFTPAREN": 182,
    "KP_RIGHTPAREN": 183,
    "KP_LEFTBRACE": 184,
    "KP_RIGHTBRACE": 185,
    "KP_TAB": 186,
    "KP_BACKSPACE": 187,
    "KP_A": 188,
    "KP_B": 189,
    "KP_C": 190,
    "KP_D": 191,
    "KP_E": 192,
    "KP_F": 193,
    "KP_XOR": 194,
    "KP_POWER": 195,
    "KP_PERCENT": 196,
    "KP_LESS": 197,
    "KP_GREATER": 198,
    "KP_AMPERSAND": 199,
    "KP_DBLAMPERSAND": 200,
    "KP_VERTICALBAR": 201,
    "KP_DBLVERTICALBAR": 202,
    "KP_COLON": 203,
    "KP_HASH": 204,
    "KP_SPACE": 205,
    "KP_AT": 206,
    "KP_EXCLAM": 207,
    "KP_MEMSTORE": 208,
    "KP_MEMRECALL": 209,
    "KP_MEMCLEAR": 210,
    "KP_MEMADD": 211,
    "KP_MEMSUBTRACT": 212,
    "KP_MEMMULTIPLY": 213,
    "KP_MEMDIVIDE": 214,
    "KP_PLUSMINUS": 215,
    "KP_CLEAR": 216,
    "KP_CLEARENTRY": 217,
    "KP_BINARY": 218,
    "KP_OCTAL": 219,
    "KP_DECIMAL": 220,
    "KP_HEXADECIMAL": 221,
    "LCTRL": 224,
    "LSHIFT": 225,
    "LALT": 226,
    "LGUI": 227,
    "RCTRL": 228,
    "RSHIFT": 229,
    "RALT": 230,
    "RGUI": 231,
    "MODE": 257,
    "SLEEP": 258,
    "WAKE": 259,
    "CHANNEL_INCREMENT": 260,
    "CHANNEL_DECREMENT": 261,
    "MEDIA_PLAY": 262,
    "MEDIA_PAUSE": 263,
    "MEDIA_RECORD": 264,
    "MEDIA_FAST_FORWARD": 265,
    "MEDIA_REWIND": 266,
    "MEDIA_NEXT_TRACK": 267,
    "MEDIA_PREVIOUS_TRACK": 268,
    "MEDIA_STOP": 269,
    "MEDIA_EJECT": 270,
    "MEDIA_PLAY_PAUSE": 271,
    "MEDIA_SELECT": 272,
    "AC_NEW": 273,
    "AC_OPEN": 274,
    "AC_CLOSE": 275,
    "AC_EXIT": 276,
    "AC_SAVE": 277,
    "AC_PRINT": 278,
    "AC_PROPERTIES": 279,
    "AC_SEARCH": 280,
    "AC_HOME": 281,
    "AC_BACK": 282,
    "AC_FORWARD": 283,
    "AC_STOP": 284,
    "AC_REFRESH": 285,
    "AC_BOOKMARKS": 286,
    "SOFTLEFT": 287,
    "SOFTRIGHT": 288,
    "CALL": 289,
    "ENDCALL": 290,
}

config_version = 0x01010101

fullscreen = 0
vsync = 0  # will increase input delay
autoplay = 0
fps = 60
offset = 0 # in ms can be negative
load_song_menu_media = 1
image_width = 1920
image_height = 1080
music_volume = 1 # min 0 default 1
sound_effect_volume = 1 # min 0 default 1
enable_video = 1
button_for_slider = 0



gameplay_keys = {
    "Triangle":  ["W", "I"],
    "Square":    ["A", "J"],
    "Cross":     ["S", "K"],
    "Circle":    ["D", "L"],
    "L":         ["Q", "U"],
    "R":         ["E", "O"],
}


gameplay_buttons = {
    "Triangle":  ["DPAD_UP", "NORTH", "LEFT_SHOULDER", "LEFT_TRIGGER", "RIGHT_TRIGGER"],
    "Square":    ["DPAD_LEFT", "WEST", "RIGHT_SHOULDER", "LEFT_TRIGGER", "RIGHT_TRIGGER"],
    "Cross":     ["DPAD_DOWN", "SOUTH", "LEFT_SHOULDER", "LEFT_TRIGGER", "RIGHT_TRIGGER"],
    "Circle":    ["DPAD_RIGHT", "EAST", "RIGHT_SHOULDER", "LEFT_TRIGGER", "RIGHT_TRIGGER"],
    "L":         ["LEFT_STICK_LEFT", "RIGHT_STICK_LEFT"],
    "R":         ["LEFT_STICK_RIGHT", "RIGHT_STICK_RIGHT"],
}


menu_keys = {
    "Up"        : "W",
    "Left"      : "A",
    "Down"      : "S",
    "Right"     : "D",
    "Triangle"  : "I",
    "Square"    : "J",
    "Cross"     : "K",
    "Circle"    : "L",
    "L"         : "Q",
    "R"         : "E",
    "Select"    : "ESCAPE",
    "Start"     : "RETURN",
}


#For now we can't bind sticks and triggers for this
menu_buttons = {
    "Up"        : "DPAD_UP",
    "Left"      : "DPAD_LEFT",
    "Down"      : "DPAD_DOWN",
    "Right"     : "DPAD_RIGHT",
    "Triangle"  : "NORTH",
    "Square"    : "WEST",
    "Cross"     : "SOUTH",
    "Circle"    : "EAST",
    "L"         : "LEFT_SHOULDER",
    "R"         : "RIGHT_SHOULDER",
    "Select"    : "BACK",
    "Start"     : "START",
}

stick_deadzone = 26000
trigger_deadzone = 26000

touch_buttons = [
    {"Button": "DPAD_UP",       "Rect": [710,   0,   100, 100]},
    {"Button": "START",           "Rect": [910,   0,   100, 100]},
    {"Button": "DPAD_DOWN",      "Rect": [1110,  0,   100, 100]},
    {"Button": "DPAD_LEFT",         "Rect": [510,    0, 100, 100]},
    {"Button": "NORTH",           "Rect": [328,  802, 278, 278]},
    {"Button": "WEST",            "Rect": [656,  802, 278, 278]},
    {"Button": "SOUTH",           "Rect": [984,  802, 278, 278]},
    {"Button": "EAST",            "Rect": [1312, 802, 278, 278]},
    {"Button": "DPAD_RIGHT",       "Rect": [1310, 0, 100, 100]}
]

with open("PLD.config", "wb") as f:
    f.write(struct.pack('I', config_version))

    f.write(struct.pack('B', fullscreen))
    f.write(struct.pack('B', vsync))
    f.write(struct.pack('B', autoplay))
    f.write(struct.pack('i', fps))
    f.write(struct.pack('i', offset))
    f.write(struct.pack('B', load_song_menu_media))
    f.write(struct.pack('i', image_width))
    f.write(struct.pack('i', image_height))
    f.write(struct.pack('f', music_volume))
    f.write(struct.pack('f', sound_effect_volume))
    f.write(struct.pack('B', enable_video))
    f.write(struct.pack('B', button_for_slider))

    for button, key_names in gameplay_keys.items():
        for key_name in key_names:
            f.write(struct.pack('i', scancodes[key_name]))

        f.write(struct.pack('i', -1))
	
    for button, button_names in gameplay_buttons.items():
        for button_name in button_names:
            f.write(struct.pack('i', gamepad_buttons[button_name]))
        
        f.write(struct.pack('i', -1))
        
    for button, key_name in menu_keys.items():
        f.write(struct.pack('i', scancodes[key_name]))

    for button, button_name in menu_buttons.items():
        f.write(struct.pack('i', gamepad_buttons[button_name]))

    f.write(struct.pack('i', trigger_deadzone))
    f.write(struct.pack('i', stick_deadzone))

    for button in touch_buttons:
        f.write(struct.pack('i', gamepad_buttons[button["Button"]]))
        f.write(struct.pack('f', button["Rect"][0]))
        f.write(struct.pack('f', button["Rect"][1]))
        f.write(struct.pack('f', button["Rect"][2]))
        f.write(struct.pack('f', button["Rect"][3]))
    f.write(struct.pack('i', -1))
