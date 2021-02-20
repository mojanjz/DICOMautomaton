import numpy as np #importing numpy module for efficiently executing numerical operations
import matplotlib.pyplot as plt #importing the pyplot from the matplotlib library
from scipy import signal

with open('data/c_noise.txt') as f:
    lines = f.readlines()
    c_t = [float(line.split()[0]) for line in lines]
    c_i = [float(line.split()[2]) for line in lines]

# - - - # We load the data in the mat format but this code will work for any sort of time series.# - - - #
cT = np.array(c_t)
cI = np.array(c_i)

# Visualizing the original and the Filtered Time Series
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
ax.plot(cT,cI,'k-',lw=0.5)

## Filtering of the time series
fs=1/1.2 #Sampling period is 1.2s so f = 1/T

nyquist = fs / 2 # 0.5 times the sampling frequency
cutoff=0.15 # fraction of nyquist frequency
b, a = signal.butter(5, cutoff, btype='lowpass') #low pass filter

cIfilt = signal.filtfilt(b, a, cI)
cIfilt=np.array(cIfilt)
cIfilt=cIfilt.transpose()

ax.plot(cT, cIfilt, 'b', linewidth=1)
ax.set_xlabel('Time (s)',fontsize=18)
ax.set_ylabel('Contrast Enhancement Intensity',fontsize=18)
plt.show()