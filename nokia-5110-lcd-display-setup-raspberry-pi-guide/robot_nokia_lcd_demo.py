#!/usr/bin/env python3
"""
robot_nokia_lcd_demo.py
www.bluetin.io
"""

__author__ = "Mark Heywood"
__version__ = "0.1.0"
__license__ = "MIT"

from inputs import get_gamepad
from time import sleep

import Adafruit_Nokia_LCD as LCD
import Adafruit_GPIO.SPI as SPI

# Python Imaging Library (PIL)
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

# Raspberry Pi hardware SPI config:
DC = 23
RST = 24
SPI_PORT = 0
SPI_DEVICE = 0

# Hardware SPI usage:
disp = LCD.PCD8544(DC, RST, spi=SPI.SpiDev(SPI_PORT, SPI_DEVICE, max_speed_hz=4000000))

# Dictionary of game controller buttons we want to include.
controller_input = {'ABS_X': 0, 'ABS_RZ': 0, 'BTN_SOUTH': 0, 'BTN_WEST': 0}

#-----------------------------------------------------------

def gamepad_update():
    # Code execution stops at the following line until gamepad event occurs.
    events = get_gamepad()
    return_code = 'No Match'
    for event in events:
        event_test = controller_input.get(event.code, 'No Match')
        if event_test != 'No Match':
            controller_input[event.code] = event.state
            return_code = event.code
        else:
            return_code = 'No Match'

    return return_code

#-----------------------------------------------------------

def drive_control():
    # Function to drive robot motors
    print('Drive and Speed --> {} || Steering Angle --> {}' .format(controller_input['ABS_RZ'], controller_input['ABS_X']) )
	
    # Create blank image for drawing.
    # Make sure to create image with mode '1' for 1-bit color.
    image = Image.new('1', (LCD.LCDWIDTH, LCD.LCDHEIGHT))

    # Get drawing object to draw on image.
    draw = ImageDraw.Draw(image)
    # Draw a white filled box to clear the image.
    draw.rectangle((0,0,LCD.LCDWIDTH,LCD.LCDHEIGHT), outline=255, fill=255)

    # Load default font.
    font = ImageFont.load_default()

    # Write some text.
    draw.text((1,5), 'Speed > {}' .format(controller_input['ABS_RZ']), font=font)
    draw.text((1,15), 'Steering > {}' .format(controller_input['ABS_X']), font=font)

    # Display image.
    disp.image(image)
    disp.display()

#-----------------------------------------------------------

def fire_nerf_dart():
    # Function to fire Nerf dart gun on the robot
    print('Fire Nerf Dart --> {}' .format(controller_input['BTN_SOUTH']) )

#-----------------------------------------------------------

def led_beacon():
    # Function to switch led beacon on/off on the robot
    print('Switch LED Beacon --> {}' .format(controller_input['BTN_WEST']) )

#-----------------------------------------------------------

def splash_screen():
    # Show splash screen at start of program and delay for five seconds.
    
    # Alternatively load a different format image, resize it, and convert to 1 bit color.
    image = Image.open('bluetin-mini-lcd-title.png').resize((LCD.LCDWIDTH, LCD.LCDHEIGHT), Image.ANTIALIAS).convert('1')

    # Display image.
    disp.image(image)
    disp.display()
    
    sleep(5)

#-----------------------------------------------------------

def main():
    """ Main entry point of the app """
    
    # Initialize library.
    disp.begin(contrast=60)

    # Clear display.
    disp.clear()
    disp.display()

    # Show splash screen. The screen will update after controller input.
    splash_screen()

    while 1:
        # Get next controller Input
        control_code = gamepad_update()
        
        # Gamepad button filter
        if control_code == 'ABS_X' or control_code == 'ABS_RZ':
            # Drive and steering
            drive_control()
        elif control_code == 'BTN_SOUTH':
            # Fire Nerf dart button
            fire_nerf_dart()
        elif control_code == 'BTN_WEST':
            # Switch the LED Beacon
            led_beacon()

#-----------------------------------------------------------

if __name__ == "__main__":
    """ This is executed when run from the command line """
    main()
