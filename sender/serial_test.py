import serial
import threading
import mido

try:
    ser = serial.Serial('COM5', baudrate=115200)  # open serial port
except Exception:
    print("Error Opening Serial.")

mid = mido.MidiFile('song/John Lee Hooker - Boom Boom Boom.mid')
length = mid.length
time = 0.0
for msg in mid.play():
    time = time + msg.time
    if (msg.type == "note_on"): # control_change, program_change, note_on
        if (msg.channel == 2):
            b = bytearray([0 | 0x80, msg.note, msg.velocity])
            print("channel: %d, note: %d, vel: %d"%(msg.channel, msg.note, msg.velocity))
            ser.write(b)
        if (msg.channel == 9):
            b = bytearray([msg.channel | 0x80, msg.note, msg.velocity])
            print("channel: %d, note: %d, vel: %d"%(msg.channel, msg.note, msg.velocity))
            ser.write(b) 
