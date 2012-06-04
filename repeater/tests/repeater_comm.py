

import serial
import struct
import time
import sys

ser = serial.Serial('/dev/ttyACM0', 9600 )
data = struct.pack("<5B", 0xAA, 0x55, 0, 1, 0);

while 1: 
#   ser.write(data)
#   time.sleep(.5)

   # Read the preamble first to verify we are getting 
   # good data
 #  recv_data = ser.read(1);
 #  preamble_A = struct.unpack("B", recv_data[:1]);
 #  if preamble_A[0] != 0xAA:
 #     continue;

   recv_data = ser.read(1);
   preamble_B = struct.unpack("B", recv_data[:1]);
   print preamble_B
 #  if preamble_B[0] != 85:
 #     continue;

 #  recv_data = ser.read(27)
 #  print struct.unpack("<19B8c", recv_data[:27]);
