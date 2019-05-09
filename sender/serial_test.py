import serial
import threading
import mido

try:
    ser = serial.Serial('COM3', baudrate=115200)  # open serial port
except Exception:
    print("Error Opening Serial.")

mid = mido.MidiFile('sender/song/John Lee Hooker - Boom Boom Boom.mid')
length = mid.length
time = 0.0
for msg in mid.play():
    time = time + msg.time
    if (msg.type == "note_on"): # control_change, program_change, note_on
        if (msg.channel == 1):
            b = bytearray([1 | 0x80, msg.note, msg.velocity])
            print("channel: %d, note: %d, vel: %d"%(msg.channel, msg.note, msg.velocity))
            print('1',time.time())
            ser.write(b)
            print('2',time.time())
        if (msg.channel == 0):
            b = bytearray([0 | 0x80, msg.note, msg.velocity])
            print("channel: %d, note: %d, vel: %d"%(msg.channel, msg.note, msg.velocity))
            ser.write(b) 
