# IPD 516 final project (MIDI & Sync Team)
Zheyuan Xie, Peng Li

## Overview
![system](system_diagram.png)

## Software
TODO

## Hardware
 - The transmitter module connected to PC sends MIDI bytes to four receiver modules.
 - The receiver modules process the bytes and communicate with the machatronic instrument through a TTL serial interface.

MIDI Transmitter PCB          |  MIDI Receiver PCB
:-------------------------:|:-------------------------:
![pcb_rx](pcb/tx.png)      |  ![pcb_tx](pcb/rx.png)

## Intruction for Instrument Teams
As an musical instrument team, you will get a receiver module. You should connect the UART2 port (or another UART port) of your ESP32 to the SER connector (3 male header pins on the right side of the receiver module).

The code that runs on your ESP32 should be derived from `insturment_template.ino`. The four functions you will need to implement are:
 - `init_inst`: Initialize your instrument, run only once when system boots.
 - `note_on`: The response to a MIDI NOTE_ON message. Do not use blocking functions like `delay()` here.
 - `note_off`: The response to a MIDI NOTE_OFF message. Instrument that does not sustain need not to implement this function.
 - `expected_delay`: In this function, Calculate the expected delay between executing the `note_on` and actually generating sound. This can be a constant or a function of note, velocity, and previous note played, depending on the type of your instrument.

The MIDI receiver module issues commands in real time, but in `instrument_template.ino`, each note is delayed `GLOBAL_DELAY - expected_delay()` microseconds to trigger the `note_on` funciton. The default value of `GLOBAL_DELAY` is **3 seconds**, which should give you enough time to prepare the actuator. Therefore the `expeteced_delay()` should not return a value larger than the `GLOBAL_DELAY`.
