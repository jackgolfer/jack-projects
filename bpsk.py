import math
import matplotlib.pyplot as plt
import numpy as np
import cmath

# Prompts user for inputs
filename = input("Enter name of input file: ")
my_hexa = input("Enter hex value to look for: ")
plot_rot = input("Plot rotated (y or n): ")
plot_muel = input("Plot M&M (y or n): ")

if(filename == ""):
    filename = "proj1_test_2.txt"
if(my_hexa == ""):
    my_hexa = "A4F2"

file = open(filename, "r")
fileOut = open('costasoutR.txt', "w")

d_beta = 0.001
d_alpha = 0.1
d_freq = 0
d_phase = 0

arr = []
arr2 = []
arrn = []
arr3 = []
arr4 = []
n = 1

# Loops that reads each real and imag sample and performs calculations on them
while True:
    fileLine = file.readline()

    if(fileLine == ''):
        break

    # Gets the real and imag numbers from the file
    fileLine = fileLine.split(' ')
    real = float(fileLine[0])
    imag = float(fileLine[1])

    # rotates the samples by d_phase but in radians 
    d_phase_rads = math.radians(d_phase)
    rotatedReal = (real * math.cos(d_phase_rads)) - (imag * math.sin(d_phase_rads))
    rotatedImag = (real * math.sin(d_phase_rads)) + (imag * math.cos(d_phase_rads))

    # saves the rotated samples for plotting
    arr.append(rotatedReal)
    arr2.append(rotatedImag)
    arrn.append(n)
    n = n + 1

    # saves the rotated real sample to file
    fileOut.write(str(rotatedReal) + '\n')

    # gets the difference in the sample 
    theta = math.atan(rotatedImag/rotatedReal)
    theta = math.degrees(theta)
    phase_error = theta * -1

    # calculates d_freq and d_phase
    d_freq += d_beta * phase_error
    if(d_freq > 360):
        d_freq -= 360
    d_phase += d_freq + (d_alpha * phase_error)
    if(d_phase > 360):
        d_phase -= 360

file.close()
fileOut.close()


# logic for plotting
if(plot_rot == 'y' or plot_muel == 'y'):
    realLine = plt.plot(arrn, arr, linewidth = '0.5')
 

if(plot_rot == 'y'):    
    imagLine = plt.plot(arrn, arr2, linewidth = '0.5', color = 'r')

file = open('costasoutR.txt', "r")
fileOut = open('mmdecisions.txt', "w")

d1 = 0 #dk
d2 = 0 #dk-1
d3 = 0 #dk+1
y1 = 0 #yk
y2 = 0 #yk-1
y3 = 0 #yk+1
n= -1
next = 1

# loop that checks each sample and makes decisions on it and when to take samples
while True:
    line = file.readline()
    n += 1
    if(line == ''):
        break

    # moves the data up, yk = yk-1, etc and reading the newest sample
    y3 = y1
    y1 = y2
    y2 = float(line)

    # moves the decisions up, dk = dk-1, etc
    d3 = d1
    d1 = d2
    if(y2 > 0):
        d2 = 1
    else:
        d2 = -1

    # saving data for plotting
    arr3.append(y2)
    arr4.append(next)


    # calculate the error in the sample taken
    u = (d2 - d3) * y1 + (y3 - y2) * d1
    u = u * -1

    # takes the next sample when n == next, either a 0 or 1
    if(n == round(next)):
        next = next + 10 + 0.1*u
        if(d1 == 1):
            fileOut.write(str(1) + '\n')
        else:
            fileOut.write(str(0) + '\n')
    

file.close()
fileOut.close()

# logic for plotting
if(plot_muel == 'y'):
    plt.plot(arr4, arr3, linewidth = '0.5', color = 'g')

if(plot_rot == 'y' or plot_muel == 'y'):
    plt.show()


file = open('mmdecisions.txt', "r")
bitStr = ""
finalPos = []
line = ''

# creates a string with the decisions made after finding what bit it is 
while(True):
    lastLine = line
    line = file.readline()
    if(line == ''):
        break
    if(line != lastLine):
        bitStr += '1'
    else:
        bitStr += '0'

# converts the user inputting hex value to binary
scale = 16
num_of_bits = len(my_hexa) * 4

binVal = bin(int(my_hexa, scale))[2:].zfill(num_of_bits)

file.close()

lastLoc = 0
temp = 0

# finds all the packets within the string of 1s and 0s
while True:
    temp = bitStr.find(str(binVal),temp+1,len(bitStr))
    if(temp == -1):
        break
    else:
        finalPos.append(temp)


#print the results
for x in finalPos:
    print(str(x))

if(len(finalPos) == 0):
    print("No matches found")
