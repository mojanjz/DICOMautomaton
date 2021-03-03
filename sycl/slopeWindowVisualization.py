import matplotlib.pyplot as plt
import numpy as np
from decimal import Decimal
import math

#This intakes a file called slope_window_exp.txt
#The file is in the form of:
#SlopeWindow Slope Intercept TimeMidpoint K1A K1V K2

#The c file input to the model
with open('c.txt') as f: #grab expected data
    lines = f.readlines()
    cET = [float(line.split()[0]) for line in lines]
    cEVal = [float(line.split()[2]) for line in lines]

with open('slope_window_exp.txt') as f: #grab expected data
    lines = f.readlines()
    slopeWindow = [float(line.split()[0]) for line in lines]
    slopes = [float(line.split()[1]) for line in lines]
    intercepts = [float(line.split()[2]) for line in lines]
    timeMidpoint = [float(line.split()[3]) for line in lines]
    k1A = [abs(float(line.split()[4])) for line in lines]
    k1V = [abs(float(line.split()[5])) for line in lines]
    k2 = [abs(float(line.split()[6])) for line in lines]


plt.subplot(1, 3, 1)
plt.plot(slopeWindow,k1A,'bo')
plt.title("Slope Window vs K1A")

plt.subplot(1, 3, 2)
plt.plot(slopeWindow,k1V, 'ro')
plt.title("Slope Window vs K1V")

plt.subplot(1, 3, 3)
plt.plot(slopeWindow,k2, 'go')
plt.title("Slope Window vs K2")


plt.show()
