import matplotlib.pyplot as plt
from scipy.signal import firwin
from scipy import signal


## loading data

accepted_steps_arduino = []
rejected_steps_arduino = []

with open('running_data.txt') as file:
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
                data = data[0].split(":")

                if data[0] == "STEP DETECTED":
                    accepted_steps_arduino.append(float(data[1]))
                elif data[0] == "STEP REJECTED":
                    rejected_steps_arduino.append(float(data[1]))
                else:
                    print(data[0])
                    raise Exception("should not be happening frfr")
        
## going to try and detect steps




def detect_steps(magnitudes, times, least_steps_per_minute, max_steps_per_minute):

    steps_detected = [] # stores time of when step was detected
    possible_step = None # will store a time of a possible step 

    rising_for = 0
    falling_for = 0
    sample_period = 17 # sample is roughly taken every 17 ms
    currently_falling = False
    currently_rising = False

    falling = False
    rising = False

    last_rise = None

    previous_rising_for = None
    troughs = []
    peaks = []
    rejected_steps = []
    
    for i in range(1, len(magnitudes)):
        magnitude = magnitudes[i]
        time = times[i]

        if (magnitude > magnitudes[i - 1]):
            rising_for += times[i] - times[i - 1]
            currently_rising, currently_falling = True, False
                
        elif (magnitude < magnitudes[i - 1]):
            falling_for += times[i] - times[i - 1]
            currently_falling, currently_rising = True, False
        else:
            currently_falling, currently_rising = False, False
            

        if ((falling_for >= sample_period * 2) and currently_falling): 

            if (rising): # rising -> falling
                possible_step = last_rise
                previous_rising_for = rising_for
                
                rising_for = 0

                peaks.append(times[i - 2])

            falling, rising = True, False
            
        elif ((rising_for >= sample_period * 2) and currently_rising):

            if not (previous_rising_for is None):
                step_time = previous_rising_for + falling_for

                print(f'Step Time: {step_time}')
                if step_time:
                    spm = 2 * (60 * 1000) / (step_time)

                print(f'Steps per minute: {spm}')

                if least_steps_per_minute < spm < max_steps_per_minute:
                    steps_detected.append(possible_step)
                else:
                    rejected_steps.append(possible_step)

                possible_step = None
                previous_rising_for = None

                troughs.append(times[i - 2])

            
            last_rise = time
            falling_for = 0

            rising, falling = True, False
        

        
        assert falling is (not rising) or (not rising and not falling) # initially neither will be true

    return steps_detected, peaks, troughs, rejected_steps

step_times, peaks, troughs, rejected_steps = detect_steps(filtered, times, 60, 235)
print(step_times)


    
plt.figure("Filtered Data")
plt.plot(times, filtered)

for time in step_times:
    plt.axvline(x=time, color='green', linestyle='-', alpha=1)

for time in rejected_steps:
    plt.axvline(x=time, color='red', linestyle='-', alpha=1)

#for time in troughs:
   # plt.axvline(x=time, color='yellow', linestyle='-', alpha=1)

#for time in peaks:
 #   plt.axvline(x=time, color='blue', linestyle='-', alpha=1)



plt.figure("What the arduino detected")
plt.plot(times, filtered)

for time in accepted_steps_arduino:
    plt.axvline(x=time, color='green', linestyle='-', alpha=1)

for time in rejected_steps_arduino:
    plt.axvline(x=time, color='red', linestyle='-', alpha=1)
    


plt.show()



        
            

            

        

            
                
            


