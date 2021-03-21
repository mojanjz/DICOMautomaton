import numpy as np #importing numpy module for efficiently executing numerical operations
import matplotlib.pyplot as plt #importing the pyplot from the matplotlib library
from scipy import signal
import os

def sanitizeData(directory, filename):
    with open(directory + '/' + filename) as f:
        lines = f.readlines()
        c_t = [float(line.split()[0]) for line in lines]
        c_i = [float(line.split()[2]) for line in lines]

    # - - - # We load the data in the mat format but this code will work for any sort of time series.# - - - #
    cT = np.array(c_t)
    cI = np.array(c_i)
    ## Filtering of the time series
    fs=1/1.2 #Sampling period is 1.2s so f = 1/T

    nyquist = fs / 2 # 0.5 times the sampling frequency
    cutoff=0.18 # fraction of nyquist frequency
    b, a = signal.butter(5, cutoff, btype='lowpass') #low pass filter

    cIfilt = signal.filtfilt(b, a, cI)
    cIfilt=np.array(cIfilt)
    cIfilt=cIfilt.transpose()

    # write filtered data back into a txt file
    zeroArr = np.zeros((cT.shape[0],), dtype=int)
    Cdata = np.column_stack((cT,zeroArr, cIfilt, zeroArr))

    filename = os.path.splitext(filename)[0]
    with open(directory + '/sanitized_aif_vif/' + filename + '_sanitizied.txt', 'w') as txt_file:
        txt_file.write('0.0 0 0.0 0\n')
        for row in Cdata:
            line = ' '.join(str(v) for v in row)
            txt_file.write(line + "\n") # works with any number of elements in a line
    # fig = plt.figure()
    # ax = fig.add_subplot(1, 1, 1)
    # ax.plot(cT, cIfilt, 'b', linewidth=1)
    # ax.set_xlabel('Time (s)',fontsize=18)
    # ax.set_ylabel('Contrast Enhancement Intensity',fontsize=18)
    # plt.show()

directory = 'data/real'
for filename in os.listdir(directory):
    if filename.endswith(".txt"):
        sanitizeData(directory, filename)
    else:
        continue
