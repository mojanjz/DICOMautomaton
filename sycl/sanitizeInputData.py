import numpy as np #importing numpy module for efficiently executing numerical operations
import matplotlib.pyplot as plt #importing the pyplot from the matplotlib library
from scipy import signal

with open('data/c_real.txt') as f:
    lines = f.readlines()
    c_t = [float(line.split()[0]) for line in lines]
    c_i = [float(line.split()[2]) for line in lines]

with open('data/aif_real.txt') as f:
    lines = f.readlines()
    aif_t = [float(line.split()[0]) for line in lines]
    aif_i = [float(line.split()[2]) for line in lines]

with open('data/vif_real.txt') as f:
    lines = f.readlines()
    vif_t = [float(line.split()[0]) for line in lines]
    vif_i = [float(line.split()[2]) for line in lines]
# - - - # We load the data in the mat format but this code will work for any sort of time series.# - - - #
cT = np.array(c_t)
cI = np.array(c_i)

aifT = np.array(aif_t)
aifI = np.array(aif_i)

vifT = np.array(vif_t)
vifI = np.array(vif_i)

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

aifIfilt = signal.filtfilt(b, a, aifI)
aifIfilt=np.array(aifIfilt)
aifIfilt=aifIfilt.transpose()

vifIfilt = signal.filtfilt(b, a, vifI)
vifIfilt=np.array(vifIfilt)
vifIfilt=vifIfilt.transpose()


# write filtered data back into a txt file
zeroArr = np.zeros((cT.shape[0],), dtype=int)
data = np.column_stack((cT,zeroArr, cIfilt, zeroArr))
with open('data/sanitized_c.txt', 'w') as txt_file:
    for row in data:
        line = ' '.join(str(v) for v in row)
        txt_file.write(line + "\n") # works with any number of elements in a line

# write filtered data back into a txt file
zeroArr = np.zeros((cT.shape[0],), dtype=int)
data = np.column_stack((aifT,zeroArr, aifIfilt, zeroArr))
with open('data/sanitized_aif.txt', 'w') as txt_file:
    for row in data:
        line = ' '.join(str(v) for v in row)
        txt_file.write(line + "\n") # works with any number of elements in a line

# write filtered data back into a txt file
zeroArr = np.zeros((cT.shape[0],), dtype=int)
data = np.column_stack((vifT,zeroArr, vifIfilt, zeroArr))
with open('data/sanitized_vif.txt', 'w') as txt_file:
    for row in data:
        line = ' '.join(str(v) for v in row)
        txt_file.write(line + "\n") # works with any number of elements in a line


ax.plot(cT, cIfilt, 'b', linewidth=1)
ax.set_xlabel('Time (s)',fontsize=18)
ax.set_ylabel('Contrast Enhancement Intensity',fontsize=18)
plt.show()