import matplotlib.pyplot as plt
from scipy.signal import firwin
from scipy import signal

accepted_steps = []
rejected_steps = []

with open('test_run_arduino_idle.txt') as file:
    text = file.read()

    groups_of_three = text.split("\n")

    magnitude, filtered, times = [], [], []

    for data_line in groups_of_three:
        
        if data_line:

            
            data = data_line.split(", ")

            if len(data) == 3:
                
                m, f, t = data
       
                magnitude.append(float(m))
                filtered.append(float(f))
                times.append(float(t))
            else:
                data = data.split(":")

                if data[0] == "DETECTED STEP":
                    accepted_steps.append(float(data[1]))
                elif data[1] == "REJECTED STEP":
                    rejected_steps.append(float(data[1]))
                else:
                    raise Exception("wtf")



times = [i/1000 for i in times] # converts to seconds


plt.figure('Unfiltered Data')
plt.plot(times, magnitude)

plt.figure('Filtered Data')
plt.plot(times, filtered)

coefficients = firwin(150, [0.5, 2], pass_zero=False, fs=58.26130843479907)
FIRacceleration = signal.lfilter(coefficients, 1.0, magnitude)

plt.figure('Filtered Data (Should be the Same')
plt.plot(times, FIRacceleration)

plt.show()

different = 0
for i in range(len(filtered)):
    if filtered[i] != FIRacceleration[i]:
        print(filtered[i]-FIRacceleration[i])

print(f'Amount that are different: {different}')
