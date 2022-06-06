import serial
import tkinter as tk
import threading

#state is global due to threads
global state
ser = serial.Serial('COM4', 9600)

#Thread that is just a loop polling for the state from the board to print and update GUI button
def loop():
    while(True):
        state = str(int.from_bytes(ser.read(1), byteorder="big"))
        print(state)
        statebutton.config(text=state)
        
        
#gui setup
root = tk.Tk()
topFrame = tk.Frame(root)
topFrame.pack()
bottomFrame= tk.Frame(root)
bottomFrame.pack(side=tk.BOTTOM)
root.geometry("400x200")

#sends + character to board to update 
def incrementState():
    ser.write(bytes('+', "utf-8"))
    
#sends - character to board to update 
def decrementState():
    ser.write(bytes('-', "utf-8"))
    
#GUI setup
button1 = tk.Button(topFrame, text="Next", fg="purple1",command = incrementState,font=("Comic Sans MS",20))
button2 = tk.Button(topFrame, text="Previous", fg="hot pink",command = decrementState,font=("Comic Sans MS",20))
statebutton = tk.Button(topFrame,text='0', fg= "lime", font=("Comic Sans MS",20))
button1.pack()
button2.pack()
statebutton.pack()




t1 = threading.Thread(target = loop, args=(),daemon=True)
t1.start()
root.mainloop()





    