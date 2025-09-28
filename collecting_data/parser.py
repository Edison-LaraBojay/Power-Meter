import struct


'''
rewrite at some point
'''


def get_data(data_file, step_file=None):

    with open(data_file) as file:
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
        assert (times[i] - times[i - 1]) < 70

    times = [time/1000 for time in times] # converts ms to seconds for graphing purposes


    if not (step_file is None):
        
        with open(step_file) as file:
            step_times = file.read().split(", ") # splits data by comma delimiter 
            step_times = [float(i) for i in step_times] # converts them all to floats (prev string)

        return (times, axValues, ayValues, azValues, gxValues, gyValues, gzValues, step_times)
    else:
        return (times, axValues, ayValues, azValues, gxValues, gyValues, gz)
    
    






