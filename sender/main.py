import mido
import serial
import remi.gui as gui
from remi import start, App
import os
import threading

CHANNEL_REMAP = [2] # map channel to 0

class MyApp(App):
    def __init__(self, *args):
        super(MyApp, self).__init__(*args)
    
    def onresize(self, emitter, w, h):
        # print(emitter, w, h)
        if int(w) > 450:
            self.container.set_size(450, 540)
        else:
            self.container.set_size(int(w), int(int(w)*(6.0/5.0)))

    def main(self):
        container = gui.VBox(width='450px', height='540px')

        # Title
        self.lbl = gui.Label("Prof. Yim's Robotic Band")
        self.lbl.style['font-size'] = '20px'
        container.append(self.lbl)

        # Serial Port Selection
        self.select_port_hbox = gui.HBox(width='100%')
        self.select_port_hbox.style['font-size'] = '16px'
        self.select_port_label = gui.Label('Select Port:', width='40%')
        self.select_port_dropdown = gui.DropDown()
        self.select_port_refresh_btn = gui.Button(text='R', width='10%', height='80%')
        self.port_list = ['COM%s' % (i + 1) for i in range(256)]
        for port in self.port_list:
            self.select_port_dropdown.append(port)
        self.select_port_hbox.append(self.select_port_label)
        self.select_port_hbox.append(self.select_port_dropdown)
        self.select_port_hbox.append(self.select_port_refresh_btn)
        container.append(self.select_port_hbox)

        # Music Selection
        self.select_music_hbox = gui.HBox(width='100%')
        self.select_music_hbox.style['font-size'] = '16px'
        self.select_music_label = gui.Label('Select Music:', width='40%')
        self.select_music_dropdown = gui.DropDown()
        self.select_music_refresh_btn = gui.Button(text='R', width='10%', height='80%')
        self.music_list = []
        for filename in os.listdir("song"):
            self.music_list.append(gui.DropDownItem(filename))
            self.select_music_dropdown.append(self.music_list[-1])
        self.select_music_hbox.append(self.select_music_label)
        self.select_music_hbox.append(self.select_music_dropdown)
        self.select_music_hbox.append(self.select_music_refresh_btn)
        container.append(self.select_music_hbox)

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
        self.table = gui.TableWidget(17, 4, True, True, width='100%')
        self.table.style['font-size'] = '14px'
        self.table.style['align-items'] = 'center'   # TODO: text alignment
        self.table.item_at(0,0).set_text("Channel")
        self.table.item_at(0,1).set_text("Note")
        self.table.item_at(0,2).set_text("Velocity")
        self.table.item_at(0,3).set_text("Time")
        for i in range(16):
            self.table.item_at(i+1, 0).set_text("%d"%(i))
        container.append(self.table)
        
        # MIDI Parser Thread
        self.mthread = None
        self.mthread_stop = False

        # returning the root widget
        self.container = container
        return self.container

    def btn_pressed(self, widget):
        if self.mthread is not None:
            self.mthread_stop = True
            self.mthread.join()
            self.mthread = None
            self.bt.set_text("Start")
            self.info.set_text('Stopped')
        else:
            self.mthread = threading.Thread(None, self.midi_thread, None)
            self.mthread_stop = False
            self.mthread.start()
            self.bt.set_text("Stop")
        
    
    def midi_thread(self):
        print("midi thread starting")
        for i in range(16):
            for j in range(3):
                self.table.item_at(i+1,j+1).set_text("")

        try:
            ser_port = self.select_port_dropdown.get_value()
            ser = serial.Serial(ser_port, baudrate=115200)  # open serial port
        except Exception:
            print("Error Opening Serial.")
            ser = None

        filename = self.select_music_dropdown.get_value()
        self.info.set_text("Loading: " + filename)
        self.bt.set_enabled(False)
        mid = mido.MidiFile("song/" + filename)
        length = mid.length
        time = 0.0
        for msg in mid.play():
            if self.mthread_stop:
                print("midi thread terminating")
                return
            time = time + msg.time
            if (msg.type == "note_on"):
                self.bt.set_enabled(True)
                self.info.set_text("Playing: " + filename) # {Channel:%d, Note: %d, Time: %.2f}"%(msg.channel, msg.note, time))
                self.table.item_at(msg.channel+1,1).set_text("%d"%msg.note)
                self.table.item_at(msg.channel+1,2).set_text("%d"%msg.velocity)
                self.table.item_at(msg.channel+1,3).set_text("%.2f s"%time)
                if ser is not None:
                    if (msg.channel in CHANNEL_REMAP):
                        b = bytearray([0, msg.note, msg.velocity,ord('\n'),ord('\r')])
                        print("channel: %d, note: %d, vel: %d"%(msg.channel, msg.note, msg.velocity))
                        ser.write(b)
                    else:
                        b = bytearray([msg.channel , msg.note, msg.velocity,ord('\n'),ord('\r')])
                        print("channel: %d, note: %d, vel: %d"%(msg.channel, msg.note, msg.velocity))
                        ser.write(b)
                self.progress.set_value(int(1000*(time/length)))
                self.do_gui_update()
        self.bt.set_text("Start")
        self.info.set_text('Stopped')
        return
    
    def on_close(self):
        print("closing...")
        if self.mthread is not None:
            self.mthread_stop = True
            self.mthread.join()
            print("force stopping midi thread")
        return super().on_close()

if __name__ == "__main__":
    start(MyApp)
