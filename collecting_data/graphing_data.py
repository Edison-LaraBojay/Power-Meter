import parser
import matplotlib.pyplot as plt
import numpy as np
from scipy import signal
from scipy.signal import firwin

times, ax, ay, az, gx, gy, gz, step_times = parser.get_data('second_test_run.txt', 'step_times_second_test_run.txt')

figure = plt.figure()

# calculates magnitude using basic formula
magnitudeAcceleration = [(ax[i]**2 + ay[i]**2 + az[i]**2)**0.5 for i in range(len(az))]


# plotting acceleration magnitude
plt.figure('acceleration magnitude vs time')
plt.plot(times, magnitudeAcceleration)
for time in step_times:
    plt.axvline(x=time, color='red', linestyle='--', alpha=0.2)
plt.xlabel('time (s)')
plt.ylabel('magnitude of acceleration')
plt.title('acceleration vs time')

# calculating average fs (not always consistently the same)
input_signal = np.array(magnitudeAcceleration)
differences = np.diff(times)
avg_fs = 1/np.mean(differences)

# calculates coefficients and applies FIR filter
coefficients = firwin(150, [0.5, 2], pass_zero=False, fs=avg_fs)
print(avg_fs)
print([float(i) for i in coefficients])
FIRacceleration = signal.lfilter(coefficients, 1.0, magnitudeAcceleration)



# plots acceleration
plt.figure('acceleration IIR vs time')
plt.plot(times, FIRacceleration)

for time in step_times:
    plt.axvline(x=time, color='red', linestyle='--', alpha=0.2)

plt.title('acceleration IIR vs time')


def rateOfChange(signal, times):
    changes = []
    for i in range(1, len(signal) - 1):
        changes.append((signal[i+1] - signal[i-1])/(times[i+1] - times[i-1]))

    return changes

# we're not graphing the first and last because we can't compute their rate of change
changes = rateOfChange(FIRacceleration, times)
del times[0]
del times[-1]


plt.figure('Rate of change')
plt.plot(times, changes)

plt.show()

