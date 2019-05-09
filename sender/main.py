import mido
import serial
import remi.gui as gui
from remi import start, App
import os
import threading
from multiprocessing import Process, Pipe
import ast
import socket

channel_map_table = {
    "BEATLES_THE_-_Octopus_s_garden-mod.mid":"?",
    "Foreplay_From_Long_Time-mhy.mid":"?",
    "John Lee Hooker - Boom Boom Boom-final.mid":"0:0,1:1,8:2,9:3",
    "Led_Zeppelin_-_Good_Times_Bad_Times mod.mid":"?",
    "Scorpions_-_Rock_You_Like_a_Hurricane.mid":"?",
    "FoolInTheRainOneString.mid":"1:0,2:1,4:3,10:9"
}

try:
    os.listdir("song")
    SONG_DIR = "song/"
except:
    SONG_DIR = "sender/song/"


def midi_process(filename, com_port, pipe, channel_map):
    # print(channel_map)
    try:
        ser = serial.Serial(com_port, baudrate=115200)  # open serial port
    except Exception:
        print("Error Opening Serial.")
        ser = None
        return
    # print("midi process starting")

    mid = mido.MidiFile(SONG_DIR + filename)

    length = mid.length
    time = 0.0
    for msg in mid.play():
        time = time + msg.time
        if (msg.type == "note_on"):
            if (msg.channel == 9):
                ser.write(bytearray([msg.channel | 0x80, msg.note, msg.velocity]))
            mapped_ch = channel_map.get(msg.channel)
            if mapped_ch is not None:
                pipe.send([mapped_ch, msg.note, msg.velocity, time, 1000*(time/length)])
                ser.write(bytearray([mapped_ch | 0x80, msg.note, msg.velocity]))
    return

class MyApp(App):
    def __init__(self, *args):
        super(MyApp, self).__init__(*args)
    
    def onresize(self, emitter, w, h):
        # print(emitter, w, h)
        if int(w) > 450:
            self.container.set_size(450, 300)
        else:
            self.container.set_size(int(w), int(int(w)*(2.0/3.0)))

    def main(self):
        container = gui.VBox(width='450px', height='300px')

        # Title
        self.lbl = gui.Label("Prof. Yim's Robotic Band")
        self.lblX = gui.Label("(X)")
        self.lblX.onclick.connect(self.btn_exited)
        self.lbl.style['font-size'] = '20px'
        container.append(gui.HBox([self.lbl, self.lblX]))

        # Serial Port Selection
        self.select_port_hbox = gui.HBox(width='100%')
        self.select_port_hbox.style['font-size'] = '16px'
        self.select_port_label = gui.Label('Select Port:', width='40%')
        self.select_port_dropdown = gui.DropDown()
        # self.select_port_refresh_btn = gui.Button(text='R', width='10%', height='80%')
        self.port_list = ['COM%s' % (i + 1) for i in range(16)]
        for port in self.port_list:
            try:
                ser = serial.Serial(port, baudrate=115200)  # open serial port
                ser.close()
                self.select_port_dropdown.append(port)
            except Exception:
                pass
        self.select_port_hbox.append(self.select_port_label)
        self.select_port_hbox.append(self.select_port_dropdown)
        # self.select_port_hbox.append(self.select_port_refresh_btn)
        container.append(self.select_port_hbox)

        # Music Selection
        self.select_music_hbox = gui.HBox(width='100%')
        self.select_music_hbox.style['font-size'] = '16px'
        self.select_music_label = gui.Label('Select Music:', width='40%')
        self.select_music_dropdown = gui.DropDown()
        # self.select_music_refresh_btn = gui.Button(text='R', width='10%', height='80%')
        self.music_list = []
        for filename in os.listdir(SONG_DIR):
            if not os.path.isdir(SONG_DIR+filename):
                self.music_list.append(gui.DropDownItem(filename))
                self.select_music_dropdown.append(self.music_list[-1])
        self.select_music_hbox.append(self.select_music_label)
        self.select_music_hbox.append(self.select_music_dropdown)
        # self.select_music_hbox.append(self.select_music_refresh_btn)
        container.append(self.select_music_hbox)
        self.select_music_dropdown.onchange.connect(self.update_channel_assignment)

        # channel mapping
        channel_map_Hbox = gui.HBox(width='100%')
        channel_map_Hbox.style['font-size'] = '16px'
        self.channel_map_textbox = gui.TextInput()
        self.channel_map_textbox.set_value("0:0,1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9")
        channel_map_Hbox.append([gui.Label("CH Map:", width='40%'), self.channel_map_textbox])
        container.append(channel_map_Hbox)

        # Play/Stop Button
        self.bt = gui.Button('Start', width = '50%', height = '50px')
        self.bt.onclick.connect(self.btn_pressed)
        self.bt.style['font-size'] = '25px'
        container.append(self.bt)

        # Play Information
        self.info = gui.Label('-')
        container.append(self.info)
        self.progress = gui.Progress(_max=1000, width='100%')
        container.append(self.progress)
        self.table = gui.TableWidget(5, 4, True, True, width='100%')
        self.table.style['font-size'] = '14px'
        self.table.style['align-items'] = 'center'   # TODO: text alignment
        self.table.item_at(0,0).set_text("Channel")
        self.table.item_at(0,1).set_text("Note")
        self.table.item_at(0,2).set_text("Velocity")
        self.table.item_at(0,3).set_text("Time")
        # for i in range(4):
        #     self.table.item_at(i+1, 0).set_text("%d"%(i))
        self.table.item_at(1,0).set_text("CH0 - Piano")
        self.table.item_at(2,0).set_text("CH1 - Bass")
        self.table.item_at(3,0).set_text("CH2 - Guitar")
        self.table.item_at(4,0).set_text("CH3 - Drum")
        container.append(self.table)
        
        # MIDI Parser Thread
        self.mprocess = None
        self.mthread = None
        self.is_playing = False
        self.pipe = None

        # returning the root widget
        self.container = container
        self.server_close = False
        return self.container

    def btn_exited(self,widget):
        self.server_close = True
        self.close()

    def btn_pressed(self, widget):
        if self.mprocess is not None:
            # print("Stop")
            self.info.set_text("-")
            self.is_playing = False
            self.mthread.join()
            self.mthread = None
        else:
            # print("Start")
            try:
                ser = serial.Serial(self.select_port_dropdown.get_value(), baudrate=115200)  # open serial port
                ser.close()
            except Exception:
                print("Error Opening Serial.")
                self.info.set_text("Cannot Open Serial Port")
                return
            tx, rx = Pipe()         # open a pipe for inter-process communication
            self.pipe = (tx, rx)
            self.is_playing = True
            channel_map = ast.literal_eval('{' +self.channel_map_textbox.get_value() + '}')
            self.mthread = threading.Thread(None, self.gui_thread, None)
            self.mprocess = Process(target = midi_process, \
                args=(self.select_music_dropdown.get_value(), \
                    self.select_port_dropdown.get_value(), \
                    self.pipe[0], channel_map))
            self.mprocess.start()
            self.mthread.start()
            self.bt.set_text("Stop")
            self.info.set_text("Playing: "+self.select_music_dropdown.get_value())
    
    def update_channel_assignment(self, widget, value):
        self.channel_map_textbox.set_text(channel_map_table[value])
    
    def gui_thread(self):
        for i in range(4):
            for j in range(3):
                self.table.item_at(i+1,j+1).set_text("")
        while(self.is_playing == True):
            if (self.pipe[1].poll()):
                recv = self.pipe[1].recv()
                channel, note, velocity = recv[0], recv[1], recv[2]
                time, progress = recv[3], recv[4]
                if (velocity > 0) and (channel <= 3):
                    self.table.item_at(channel+1,1).set_text("%d"%note)
                    self.table.item_at(channel+1,2).set_text("%d"%velocity)
                    self.table.item_at(channel+1,3).set_text("%.2f s"%time)
                self.progress.set_value(progress)
            if (not self.mprocess.is_alive()):      # if the midi process reaches its end
                self.mprocess.join()
                self.is_playing = False
        if (self.mprocess.is_alive()):              # if the midi process is still playing 
            self.mprocess.terminate()
        self.pipe[0].close()    # close sender pipe connections
        self.pipe[1].close()    # close receiver pipe connections
        self.mprocess = None
        self.bt.set_text("Start")
    
    def on_close(self):
        print("exiting...")
        if (self.is_playing):
            self.is_playing = False
            self.mthread.join()
        super(MyApp, self).on_close()

if __name__ == "__main__":
    start(MyApp, standalone = True, width=450, height=300, resizable=False, title="MIDI Team")
