#!/usr/bin/env python3
"""
uart_buttons.py - Simulate STM32H747I-DISCO board buttons from the PC keyboard.

Key-pad mapping:
  8 / Arrow-Up    -> Joystick UP
  4 / Arrow-Left  -> Joystick LEFT
  2 / Arrow-Down  -> Joystick DOWN
  6 / Arrow-Right -> Joystick RIGHT
  5               -> Joystick CENTER (select)
  Enter           -> Push button (toggle navigation mode)
  - / Ctrl-C      -> Quit

Usage:
  python uart_buttons.py COM7    # specify COM port

Requirements:
  pip install pyserial
"""

import sys
import serial
import msvcrt   

# Extended key codes returned by msvcrt.getch() for arrow keys (two-byte sequence)
_ARROW_UP    = b'H'
_ARROW_DOWN  = b'P'
_ARROW_LEFT  = b'K'
_ARROW_RIGHT = b'M'

# encode input 
_CMD = {
    '8': b'8',  # JOY_UP
    '2': b'2',  # JOY_DOWN
    '4': b'4',  # JOY_LEFT
    '6': b'6',  # JOY_RIGHT
    '5': b'5',  # JOY_SEL (center)
}

_LABEL = {
    b'8': 'JOY_UP',
    b'2': 'JOY_DOWN',
    b'4': 'JOY_LEFT',
    b'6': 'JOY_RIGHT',
    b'5': 'JOY_SEL (center)',
    b'\r': 'PUSH BUTTON (nav toggle)',
}


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else "COM7"

    try:
        ser = serial.Serial(port, baudrate=115200, timeout=0.05)
    except serial.SerialException as exc:
        print(f"Error opening {port}: {exc}")
        print("Usage: python uart_buttons.py <COM_PORT>")
        sys.exit(1)

    print(f"Connected to {port} at 115200 baud.")
    print("Keypad: 8=up  2=down  4=left  6=right  5=select  Enter=nav-toggle  -=quit")
    print("Arrow keys also work for joystick directions.\n")

    try:
        while True:
            raw = msvcrt.getch()

            # Arrow keys produce a two-byte sequence: 0xe0 or 0x00 followed by the code
            if raw in (b'\xe0', b'\x00'):
                raw2 = msvcrt.getch()
                if   raw2 == _ARROW_UP:    cmd = b'8'
                elif raw2 == _ARROW_DOWN:  cmd = b'2'
                elif raw2 == _ARROW_LEFT:  cmd = b'4'
                elif raw2 == _ARROW_RIGHT: cmd = b'6'
                else:                      continue
            else:
                ch = raw.decode('utf-8', errors='ignore')
                if ch == '-':
                    break
                if ch == '\r':   # Enter -> push button / nav toggle
                    cmd = b'\r'
                elif ch in _CMD:
                    cmd = _CMD[ch]
                else:
                    continue

            ser.write(cmd)
            print(f"  -> {_LABEL.get(cmd, cmd)}")

    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        print("Disconnected.")


if __name__ == "__main__":
    main()
