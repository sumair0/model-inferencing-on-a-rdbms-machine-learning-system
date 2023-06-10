# The idea is stolen from Shangyu's ICDE paper, created by Binhang June 2017
import random
import sys
import time


data_num = int(sys.argv[1])
dim = int(sys.argv[2])
blockRowSize = int(sys.argv[3])
blockColSize = int(sys.argv[4])
assert data_num% blockRowSize==0 and dim%blockColSize ==0
blockRowNum = data_num/blockRowSize
blockColNum = dim/blockColSize


#lines = open("./src/linearAlgebraDSL/TestDataGenerator/Gram_M_"+str(data_num)+"_"+str(dim)+".csv", "w")
#lines_mtd = open("./src/linearAlgebraDSL/TestDataGenerator/Gram_M_"+str(data_num)+"_"+str(dim)+".csv.mtd", "w")
fileName = "./src/linearAlgebraDSL/TestDataGenerator/Gram_M_"+str(blockRowSize)+"_"+str(blockColSize)+"_"+str(blockRowNum)+"_"+str(blockColNum)+".data"
blocks = open(fileName,"w")
code = open("./src/linearAlgebraDSL/DSLSamples/Task01_Gram_"+str(data_num)+"_"+str(dim)+"_"+str(blockRowSize)+"_"+str(blockColSize)+".pdml", "w")


print "data_num: " + str(data_num) + "	dim: " + str(dim) + "  block row size: " +str(blockRowSize) + "  block col size: " + str(blockColSize) + "  block row number: "+ str(blockRowNum) +"  block col number: "+str(blockColNum)


data = []
for i in xrange(data_num):
    row = []    
    for j in xrange(dim):
        row.append(random.random())
    data.append(row)


#for i in xrange(data_num):
#    for j in xrange(dim-1):
#        lines.write(str(data[i][j]))
#        lines.write(", ")
#    lines.write(str(data[i][dim-1]))
#    lines.write("\n")


for i in xrange(data_num/blockRowSize):
	for j in xrange(dim/blockColSize):
		blocks.write(str(i))
		blocks.write(" ")
		blocks.write(str(j))
		blocks.write(" ")
		for ii in xrange(blockRowSize):
			for jj in xrange(blockColSize):
				blocks.write(str(data[i*blockRowSize+ii][j*blockColSize+jj]))
				blocks.write(" ")
		blocks.write("\n")


#lines_mtd.write("{\"rows\": " + str(data_num) +", \"cols\": " + str(dim) + ", \"format\": \"csv\"}")
code.write("X = load("+str(blockRowSize)+","+str(blockColSize)+","+str(blockRowNum)+","+str(blockColNum)+',\"'+fileName+'\")\n')
code.write("M = X '* X\n")


#lines.close()
#lines_mtd.close()
blocks.close()
code.close()