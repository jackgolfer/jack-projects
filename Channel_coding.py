import json

def getXValues(b0,b1,b2):
    x = [0,0,0]
    x[0] = (b0 + b1 + b2) % 2
    x[1] = (b2 + b0) % 2
    x[2] = (b2 + b1) % 2
    return x

def getNewCost(oldCost,S, sample1, sample2,sample3,newNum):
    tempCost0 = oldCost
    b2 = newNum
    if(len(S) > 0):
        b1 = int(S[len(S) - 1])
    else:
        b1 = 0
    if(len(S) > 1):
        b0 = int(S[len(S) - 2])
    else:
        b0 = 0

    xVals1 = []
    xVals1 = getXValues(b0,b1,b2)
    if(xVals1[0] != sample1):
        tempCost0 += 1
    if(xVals1[1] != sample2):
        tempCost0 += 1
    if(xVals1[2] != sample3):
        tempCost0 += 1
    return tempCost0

def getNewSoftCost(oldCost,S, sample1, sample2,sample3,newNum):
    b2 = newNum
    if(len(S) > 0):
        b1 = int(S[len(S) - 1])
    else:
        b1 = 0
    if(len(S) > 1):
        b0 = int(S[len(S) - 2])
    else:
        b0 = 0

    xVals1 = []
    xVals1 = getXValues(b0,b1,b2)

    for x in range(3):
        if (xVals1[x] == 0):
            xVals1[x] = 1
        else:
            xVals1[x] = -1

    constant = .5


    if (newNum == 0):
        num = 1
    else:
        num = -1

    tempCost0 = constant * (pow(xVals1[0]+float(sample1), 2) + pow(xVals1[1] + float(sample2),2) + pow(xVals1[2] + float(sample3),2))
    tempCost0 += oldCost
    # print(str(sample1) + '|' + str(sample2) + '|' + str(sample3))
    # print((pow(num+float(sample1), 2) + pow(num + float(sample2),2) + pow(num + float(sample3),2)))
    # print(S + '|' + str(tempCost0))

    return tempCost0


file = open('config', "r")
line = file.read()

config = json.loads(line)

file.close()

file = open(config["samplesFilename"],'r')
fileOut = open(config["hdFilename"], 'w')

n = 1

S0 = ""
S1 = ""
S2 = ""
S3 = ""
S0Cost = 0
S1Cost = 0
S2Cost = 97
S3Cost = 98
b2 = 0
b1 = 0
b0 = 0
xVals1 = []

while True:
    sample1 = file.readline()
    sample2 = file.readline()
    sample3 = file.readline()

    if(sample1 == '' or sample2 == '' or sample3 == ''):
        break

    sample1 = float(sample1)
    sample2 = float(sample2)
    sample3 = float(sample3)

    if(sample1 > 0):
        hardSample1 = 1
    else:
        hardSample1 = 0
    if(sample2 > 0):
        hardSample2 = 1
    else:
        hardSample2 = 0
    if(sample3 > 0):
        hardSample3 = 1
    else:
        hardSample3 = 0


    oldS0 = S0
    oldS1 = S1
    oldS2 = S2
    oldS3 = S3
    oldS0Cost = S0Cost
    oldS1Cost = S1Cost
    oldS2Cost = S2Cost
    oldS3Cost = S3Cost




    # S0=[old S0,0] OR [old S2,0]
    tempCost0 = getNewCost(oldS0Cost,oldS0,hardSample1,hardSample2,hardSample3,0)
    tempCost1 = getNewCost(oldS2Cost,oldS2,hardSample1,hardSample2,hardSample3,0)

    if(tempCost1 < tempCost0):
        S0 = oldS2 + '0'
        S0Cost = tempCost1
    else:
        S0 = oldS0 + '0'
        S0Cost = tempCost0

    # S1=[old S0,1] OR [old S2,1]
    tempCost0 = getNewCost(oldS0Cost,oldS0,hardSample1,hardSample2,hardSample3,1)
    tempCost1 = getNewCost(oldS2Cost,oldS2,hardSample1,hardSample2,hardSample3,1)

    if(tempCost1 < tempCost0):
        S1 = oldS2 + '1'
        S1Cost = tempCost1
    else:
        S1 = oldS0 + '1'
        S1Cost = tempCost0


    # S2=[old S1,0] OR [old S3,0]
    tempCost0 = getNewCost(oldS1Cost,oldS1,hardSample1,hardSample2,hardSample3,0)
    tempCost1 = getNewCost(oldS3Cost,oldS3,hardSample1,hardSample2,hardSample3,0)

    if(tempCost1 < tempCost0):
        S2 = oldS3 + '0'
        S2Cost = tempCost1
    else:
        S2 = oldS1 + '0'
        S2Cost = tempCost0


    # S3=[old S1,1] OR [old S3,1]
    tempCost0 = getNewCost(oldS1Cost,oldS1,hardSample1,hardSample2,hardSample3,1)
    tempCost1 = getNewCost(oldS3Cost,oldS3,hardSample1,hardSample2,hardSample3,1)

    if(tempCost1 < tempCost0):
        S3 = oldS3 + '1'
        S3Cost = tempCost1
    else:

        S3 = oldS1 + '1'
        S3Cost = tempCost0

    if(n == 1):
        S2Cost = 97
        S3Cost = 98



    # print("step " + str(n) + " -- samples: [" + str(sample1) + ", " + str(sample2) + ", " + str(sample3) + "]; cost: ["
    # + str(S0Cost) + ", " + str(S1Cost) + ", " + str(S2Cost) + ", " + str(S3Cost) + "]\n"
    # + "S0:" + S0 + "\nS1:" + S1 + "\nS2:" + S2 + "\nS3:" + S3 + "\n")

    fileOut.write("step " + str(n) + ": [" + 
    str(S0Cost) + ", " + str(S1Cost) + ", " + str(S2Cost) + ", " + str(S3Cost) + "]\n" + 
    "S0:" + S0 + "\nS1:" + S1 + "\nS2:" + S2 + "\nS3:" + S3 + "\n")
    n += 1



print("step " + str(n-1) + ": ["
+ str(S0Cost) + ", " + str(S1Cost) + ", " + str(S2Cost) + ", " + str(S3Cost) + "]\n"
+ "S0:" + S0 + "\nS1:" + S1 + "\nS2:" + S2 + "\nS3:" + S3 + "\n")

minCost = min(S0Cost,S1Cost,S2Cost,S3Cost)

if(S0Cost == minCost and S0[len(S0)-1] == '0' and S0[len(S0)-2] == '0'):
    finalS = S0
elif(S1Cost == minCost and S1[len(S1)-1] == '0' and S1[len(S1)-2] == '0'):
    finalS = S1
elif(S2Cost == minCost and S2[len(S2)-1] == '0' and S2[len(S2)-2] == '0'):
    finalS = S2
elif(S3Cost == minCost and S3[len(S3)-1] == '0' and S3[len(S3)-2] == '0'):
    finalS = S3
else:
    finalS = S3

file.close()
fileOut.close()

file = open(config["dataFilename"], 'r')
errNum = 0
n = 0
while True:
    line = file.readline()
    if(line == ''):
        break
    if(finalS[n] != line[0]):
        errNum += 1
    n += 1

file.close()

for x in range(64):
    print(finalS[x] + ' ', end ='')
print("\nerrnum = " + str(errNum) + '\n')


######################################################################################################
#   Soft decoding
######################################################################################################

file = open(config["samplesFilename"],'r')
fileOut = open(config["sdFilename"], 'w')

n = 1

S0 = ""
S1 = ""
S2 = ""
S3 = ""
S0Cost = 0
S1Cost = 0
S2Cost = 97
S3Cost = 98
b2 = 0
b1 = 0
b0 = 0
xVals1 = []

while True:
    sample1 = file.readline()
    sample2 = file.readline()
    sample3 = file.readline()

    if(sample1 == '' or sample2 == '' or sample3 == ''):
        break

    softSample1 = float(sample1)
    softSample2 = float(sample2)
    softSample3 = float(sample3)

    oldS0 = S0
    oldS1 = S1
    oldS2 = S2
    oldS3 = S3
    oldS0Cost = S0Cost
    oldS1Cost = S1Cost
    oldS2Cost = S2Cost
    oldS3Cost = S3Cost




    # S0=[old S0,0] OR [old S2,0]
    tempCost0 = getNewSoftCost(oldS0Cost,oldS0,softSample1,softSample2,softSample3,0)
    tempCost1 = getNewSoftCost(oldS2Cost,oldS2,softSample1,softSample2,softSample3,0)

    if(tempCost1 < tempCost0):
        S0 = oldS2 + '0'
        S0Cost = tempCost1
    else:
        S0 = oldS0 + '0'
        S0Cost = tempCost0

    # S1=[old S0,1] OR [old S2,1]
    tempCost0 = getNewSoftCost(oldS0Cost,oldS0,softSample1,softSample2,softSample3,1)
    tempCost1 = getNewSoftCost(oldS2Cost,oldS2,softSample1,softSample2,softSample3,1)

    if(tempCost1 < tempCost0):
        S1 = oldS2 + '1'
        S1Cost = tempCost1
    else:
        S1 = oldS0 + '1'
        S1Cost = tempCost0


    # S2=[old S1,0] OR [old S3,0]
    tempCost0 = getNewSoftCost(oldS1Cost,oldS1,softSample1,softSample2,softSample3,0)
    tempCost1 = getNewSoftCost(oldS3Cost,oldS3,softSample1,softSample2,softSample3,0)

    if(tempCost1 < tempCost0):
        S2 = oldS3 + '0'
        S2Cost = tempCost1
    else:
        S2 = oldS1 + '0'
        S2Cost = tempCost0


    # S3=[old S1,1] OR [old S3,1]
    tempCost0 = getNewSoftCost(oldS1Cost,oldS1,softSample1,softSample2,softSample3,1)
    tempCost1 = getNewSoftCost(oldS3Cost,oldS3,softSample1,softSample2,softSample3,1)

    if(tempCost1 < tempCost0):
        S3 = oldS3 + '1'
        S3Cost = tempCost1
    else:

        S3 = oldS1 + '1'
        S3Cost = tempCost0

    if(n == 1):
        S2Cost = 97
        S3Cost = 98

    fileOut.write("step " + str(n) + ": [" + 
    str(S0Cost) + ", " + str(S1Cost) + ", " + str(S2Cost) + ", " + str(S3Cost) + "]\n" + 
    "S0:" + S0 + "\nS1:" + S1 + "\nS2:" + S2 + "\nS3:" + S3 + "\n")
    n += 1



print("step " + str(n-1) + ": ["
+ str(S0Cost) + ", " + str(S1Cost) + ", " + str(S2Cost) + ", " + str(S3Cost) + "]\n"
+ "S0:" + S0 + "\nS1:" + S1 + "\nS2:" + S2 + "\nS3:" + S3 + "\n")

minCost = min(S0Cost,S1Cost,S2Cost,S3Cost)

if(S0Cost == minCost and S0[len(S0)-1] == '0' and S0[len(S0)-2] == '0'):
    finalS = S0
elif(S1Cost == minCost and S1[len(S1)-1] == '0' and S1[len(S1)-2] == '0'):
    finalS = S1
elif(S2Cost == minCost and S2[len(S2)-1] == '0' and S2[len(S2)-2] == '0'):
    finalS = S2
elif(S3Cost == minCost and S3[len(S3)-1] == '0' and S3[len(S3)-2] == '0'):
    finalS = S3
else:
    finalS = S3

file.close()
fileOut.close()

file = open(config["dataFilename"], 'r')
errNum = 0
n = 0
while True:
    line = file.readline()
    if(line == ''):
        break
    if(finalS[n] != line[0]):
        errNum += 1
    n += 1
file.close()
for x in range(64):
    print(finalS[x] + ' ', end ='')
print("\nerrnum = " + str(errNum))
