#!/usr/bin/env python3
"""
multithreading_gamepad.py
Mark Heywood, 2018
www.bluetin.io
10/01/2018
"""

__author__ = "Mark Heywood"
__version__ = "0.1.1"
__license__ = "MIT"


from time import sleep
from threading import Thread
import queue
import time

from inputs import get_gamepad


class ThreadedInputs:
	NOMATCH = 'No Match'
	
	def __init__(self):
		# Initialise gamepad command dictionary.
		# Add gamepad commands using the append method before executing the start method.
		self.gamepadInputs = {}
		self.lastEventCode = self.NOMATCH
		# Initialise the thread status flag
		self.stopped = False
		self.q = queue.LifoQueue()

	def start(self):
		# Start the thread to poll gamepad event updates
		t = Thread(target=self.gamepad_update, args=())
		t.daemon = True
		t.start()
		
	def gamepad_update(self):
		while True:
			# Should the thread exit?
			if self.stopped:
				return
			# Code execution stops at the following line until a gamepad event occurs.
			events = get_gamepad()
			for event in events:
				event_test = self.gamepadInputs.get(event.code, self.NOMATCH)
				if event_test != self.NOMATCH:
					self.gamepadInputs[event.code] = event.state
					self.lastEventCode = event.code
					self.q.put(event.code)

	def read(self):
		# Return the latest command from gamepad event
		if not self.q.empty():
			newCommand = self.q.get()
			while not self.q.empty():
				trashBin = self.q.get()
	
			return newCommand, self.gamepadInputs[newCommand]
		else:
			return self.NOMATCH, 0

	def stop(self):
		# Stop the game pad thread
		self.stopped = True
		
	def append_command(self, newCommand, newValue):
		# Add new controller command to the list
		if newCommand not in self.gamepadInputs:
			self.gamepadInputs[newCommand] = newValue
		else:
			print('New command already exists')
		
	def delete_command(self, commandKey):
		# Remove controller command from list
		if commandKey in self.gamepadInputs:
			del self.gamepadInputs[commandKey]
		else:
			print('No command to delete')

	def command_value(self, commandKey):
		# Get command value
		if commandKey in self.gamepadInputs:
			return self.gamepadInputs[commandKey]
		else:
			return None


def drive_control():
	# Function to drive robot motors
	print('Speed -> {} || Value -> {}'.format('ABS_RZ', gamepad.command_value('ABS_RZ')))
	print('Direction -> {} || Value -> {}'.format('ABS_X', gamepad.command_value('ABS_X')))
    

def fire_nerf_dart(commandInput, commandValue):
	# Function to fire Nerf dart gun on the robot
	print('Fire Nerf Dart -> {} Value -> {}'.format(commandInput, commandValue))


def led_beacon(commandInput, commandValue):
	# Function to switch led beacon on/off on the robot
	print('Switch LED Beacon -> {} Value -> {}'.format(commandInput, commandValue))

#-----------------------------------------------------------

# Dictionary of game controller buttons we want to include.
gamepadInputs = {'ABS_X': 128, 
				'ABS_RZ': 127, 
				'BTN_SOUTH': 0, 
				'BTN_WEST': 0,
				'BTN_START': 0}

# Initialise the gamepad object using the gamepad inputs Python package
gamepad = ThreadedInputs()

def main():
	""" Main entry point of this program """
	# Load the object with gamepad buttons we want to catch 
	for gamepadInput in gamepadInputs:
		gamepad.append_command(gamepadInput, gamepadInputs[gamepadInput])
	# Start the gamepad event update thread
	gamepad.start()

	while 1:
		#timeCheck = time.time()
		# Get the next gamepad button event
		commandInput, commandValue = gamepad.read()
		# Gamepad button command filter
		if commandInput == 'ABS_X' or commandInput == 'ABS_RZ':
			# Drive and steering
			drive_control()
		elif commandInput == 'BTN_SOUTH':
			# Fire Nerf dart button for example
			fire_nerf_dart(commandInput, commandValue)
		elif commandInput == 'BTN_WEST':
			# Switch the LED Beacon for example
			led_beacon(commandInput, commandValue)
		elif commandInput == 'BTN_START':
			# Exit the while loop - this program is closing
			break 

		sleep(0.01)
		#print(commandInput, commandValue)
		#print(1/(time.time() - timeCheck))

	# Stop the gamepad thread and close this program
	gamepad.stop()
	exit()
	
	
#-----------------------------------------------------------

if __name__ == "__main__":
	""" This is executed when run from the command line """
	main()
