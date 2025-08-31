import struct
import matplotlib.pyplot as plt

'''
rewrite at some point
'''


with open("initial_test.txt") as file:
    # example format of one line
    '''
    I	01:56:26.625	Notification received from 19b10001-e8f2-537e-4f6c-d104768a1214, value: (0x) 1D-FA-00-00-00-00-9E-3C-00-48-79-3F-00-00-EE-BC
    A	01:56:26.625	"(0x) 1D-FA-00-00-00-00-9E-3C-00-48-79-3F-00-00-EE-BC" received
    I	01:56:26.656	Notification received from 19b10001-e8f2-537e-4f6c-d104768a1214, 
    '''
    # we only care about the hex value following the 'value: (0x)'
    # so we use this to split the list, meaning the first element will be ignored
    # and the second one will contain the value we want (until it hits the next instance of value: (0x)
    # so we then must further split this by \n (because it starts a new line after the hex value
    # and take only the first part of this split (and ignore the second part)
    rawData = file.read()
    # 
    splitData = rawData.split("value: (0x) ")
    splitData.pop(0) # removes first element (useless)
    
    times = []
    axValues = []
    ayValues = []
    azValues = []
    gxValues = []
    gyValues = []
    gzValues = []

    for text in splitData:
        hexVal = text.split("\n")[0]
        hexVal = " ".join(hexVal.split("-")) # to move it from the format of FF-FF-FF to FF FF FF
        hexVal = bytes.fromhex(hexVal)
        time, ax, ay, az, gx, gy, gz = struct.unpack("<Iffffff", hexVal) # get data from hex value


        # under the assumption that this case strictly happens right after (hence the -1)
        if len(times) != 0 and time == times[-1]:
            # if we receive two of the same packet
            # (which at least in the log its being printed twice not necessarily
            # because its being received twice just how its being displayed
            # but also in that regard to ensure we're not accidentally
            # interpreting it incorrectly (would probably come from how we're splitting
            # and trying to isolate the hex 
            assert ax == axValues[-1]
            assert ay == ayValues[-1]
            assert az == azValues[-1]
            assert gx == gxValues[-1]
            assert gy == gyValues[-1]
            assert gz == gzValues[-1]
            
        else:
            if len(times) != 0:
                times.append(time - times[0]) # usings the first time recorded as zero
            else:
                times.append(time) # this is the zero reference
                
            axValues.append(ax)
            ayValues.append(ay)
            azValues.append(az)
            gxValues.append(gx)
            gyValues.append(gy)
            gzValues.append(gz)

times[0] = 0 # we needed to store time prior because we needed to find the time
# elapsed *after* but now for plotting purposes we want this to be zero

# removes the placeholder value we no longer need

# to check for latency issues but also
# to ensure the packets are being received in order (generally) and if not
# possibly an error? at least something to look into

for i in range(1, len(times)):
    assert times[i] > times[i - 1]
    assert (times[i] - times[i - 1]) < 50

figure = plt.figure()

# adds 3 subplots to a 2x2 grid
plt1 = figure.add_subplot(2, 3, 1)
plt2 = figure.add_subplot(2, 3, 2)
plt3 = figure.add_subplot(2, 3, 3)
plt4 = figure.add_subplot(2, 3, 4)
plt5 = figure.add_subplot(2, 3, 5)
plt6 = figure.add_subplot(2, 3, 6)

# plotting acceleration in x direction
plt1.plot(times, axValues)
plt1.set_xlabel('time (ms)')
plt1.set_ylabel('x-acceleration')
plt1.set_title('x vs time')

# plotting acceleration in y direction
plt2.plot(times, ayValues)
plt2.set_xlabel('time (ms)')
plt2.set_ylabel('y-acceleration')
plt2.set_title('y vs time')

# plotting acceleration in z direction
plt3.plot(times, azValues)
plt3.set_xlabel('time (ms)')
plt3.set_ylabel('z-acceleration')
plt3.set_title('z vs time')

# plotting gyroscopic x value
plt4.plot(times, gxValues)
plt4.set_xlabel('time (ms')
plt4.set_ylabel('x-gyro')
plt4.set_title('gx vs time')


plt5.plot(times, gyValues)
plt5.set_xlabel('time (ms')
plt5.set_ylabel('y-gyro')
plt5.set_title('gy vs time')


plt6.plot(times, gzValues)
plt6.set_xlabel('time (ms')
plt6.set_ylabel('z-gyro')
plt6.set_title('gz vs time')


plt.show()




