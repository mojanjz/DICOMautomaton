import matplotlib.pyplot as plt
import numpy as np
from decimal import Decimal
import math
import sys

#This intakes a file called slope_window_exp.txt
#The file is in the form of:
#SlopeWindow Slope Intercept TimeMidpoint K1A K1V K2
try:
    k1AExpected = float(sys.argv[1])
    k1VExpected = float(sys.argv[2])
    k2Expected = float(sys.argv[3])
except IndexError:
   print("Please input k1A k1V k2 Expected values")
   sys.exit(1)

k1AExpectedRatio = k1AExpected/(k1AExpected+k1VExpected)
k1VExpectedRatio = k1VExpected/(k1AExpected+k1VExpected)

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

numberExperiments = len(slopeWindow) #the number of experiments in this text file

#Force the values into lists to be able to plot
k1AExpected = [k1AExpected]*numberExperiments
k1VExpected = [k1VExpected]*numberExperiments
k2Expected = [k2Expected]*numberExperiments
k1AExpectedRatio = [k1AExpectedRatio]*numberExperiments
k1VExpectedRatio = [k1VExpectedRatio]*numberExperiments

slopeCVals = []
slopetVals = []
k1ARatios = []
k1VRatios = []

for i in range (numberExperiments):
    slopeCValsIntermediate = []
    slopetValsIntermediate = []
    for tVal in range (int(timeMidpoint[i])-10, int(timeMidpoint[i])+11):
        slopeCValsIntermediate.append(tVal*slopes[i]+intercepts[i])
        slopetValsIntermediate.append(tVal)
        tVal = tVal+1
    slopeCVals.append(slopeCValsIntermediate)
    slopetVals.append(slopetValsIntermediate)
    k1ARatios.append(k1A[i]/(k1A[i]+k1V[i]))
    k1VRatios.append(k1V[i]/(k1A[i]+k1V[i]))

plt.xlabel("Time")
plt.ylabel("Contrast Agent Intensity")
plt.title("The effect of Slope Window Size on Slope")
for i in range(len(slopeCVals)):
    # plt.plot(slopetVals,[pt[i] for pt in slopeCVals],label = 'id %s'%i)
    plt.plot(slopetVals[i],slopeCVals[i], label = 'Slope Window %s'%slopeWindow[i])
plt.legend()
plt.plot(cET,cEVal)
# plt.show()

plt.subplot(2, 3, 1)
plt.plot(slopeWindow,k1A,'bo')
plt.plot(slopeWindow, k1AExpected, 'b--')
plt.title("Slope Window vs K1A")
plt.xlabel("Slope Window")
plt.ylabel("k1A Value")

plt.subplot(2, 3, 2)
plt.plot(slopeWindow,k1V, 'ro')
plt.plot(slopeWindow, k1VExpected, 'r--')
plt.title("Slope Window vs K1V")
plt.xlabel("Slope Window")
plt.ylabel("k1V Value")

plt.subplot(2, 3, 3)
plt.plot(slopeWindow,k2, 'go')
plt.plot(slopeWindow, k2Expected, 'g--')
plt.title("Slope Window vs K2")
plt.xlabel("Slope Window")
plt.ylabel("k2 Value")

plt.subplot(2, 3, 4)
for i in range(len(slopeCVals)):
    plt.plot(slopetVals[i],slopeCVals[i], label = 'Slope Window %s'%slopeWindow[i])
plt.legend()
plt.plot(cET,cEVal)
plt.xlabel("Time")
plt.ylabel("Contrast Agent Intensity")
plt.title("The effect of Slope Window Size on Slope")

plt.subplot(2, 3, 5)
plt.plot(slopeWindow,k1AExpectedRatio, 'g--', label = 'Expected Ratio')
plt.plot(slopeWindow, k1ARatios, 'b', label = 'Actual Ratio')
plt.legend()
plt.title("The Effect of Slope Window on K1A Ratio")
plt.xlabel("Slope Window")
plt.ylabel("k1A Ratio")

plt.subplot(2, 3, 6)
plt.plot(slopeWindow,k1VExpectedRatio, 'g--', label = 'Expected Ratio')
plt.plot(slopeWindow, k1VRatios, 'b', label = 'Actual Ratio')
plt.legend()
plt.title("The Effect of Slope Window on K1V Ratio")
plt.xlabel("Slope Window")
plt.ylabel("k1V Ratio")

plt.show()
