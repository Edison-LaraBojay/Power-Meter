import serial
import keyboard

start_reading = False

ser = serial.Serial("COM4", 9600)
text =""
with open("walking_data.txt", "w") as f:
    while True:
        line = ser.readline().decode(errors="ignore").strip()

        
        if line and start_reading:
            text += line + "\n"
            print(line)

        if not start_reading and keyboard.is_pressed("s"):
            start_reading = True
            
        elif keyboard.is_pressed(" "):
            f.write(text)
            break
            

        
            

        
