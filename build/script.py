import subprocess
import re
import statistics
import os
import math


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# print("Cleaning...")
#make = subprocess.getoutput("make clean")
# print(make)


tests = 5

days = [30, 180, 365]
threads = [0, 1, 2, 4]

datasetSize = [4039, 26518, 36692]
datasetEdges = [88234, 65369, 367662]

datasets = ["../datasets/facebook.txt",
            "../datasets/gnutella.txt", "../datasets/enron.txt"]

seeds = ["../datasets/fbSeed.txt",
         "../datasets/gnuSeed.txt", "../datasets/enronSeed.txt"]


# Parallelism Measuerements
for i in range(len(datasets)):

    print("---------------------------------------------------------------")
    print("---------------------------------------------------------------")

    print("Dataset path:", "\"", datasets[i], "\"")

    for j in range(len(days)):

        print("--\nDays:", days[j])

        avg = []
        stddev = []
        speedups = []

        for k in range(len(threads)):

            if threads[k] == 0:
                make = subprocess.getoutput("make nomp")
                # print(make)

            for m in range(tests):
                command = "./epidemic -f " + \
                    datasets[i] + " -s " + seeds[i] + " -d " + \
                    str(days[j]) + " -t " + str(threads[k]) + " -c"
                # print(command)

                out = subprocess.getoutput(command)
                # print(out)

            values = []
            with open('timeCalculations.txt', 'r+') as f:
                for line in f:
                    if line:
                        values.append(float(line.strip()))

            avrg = "{:.5f}".format(sum(values) / len(values))
            dev = "{:.5f}".format(statistics.stdev(values))

            avg.append(float(avrg))
            stddev.append(float(dev))

            print("--Thread(s) #", threads[k], values,
                  "=> avg: ", avrg, "s, stdev: ", dev, " s")

            os.remove("timeCalculations.txt")

            if threads[k] == 0:
                make = subprocess.getoutput("make")
                # print(make)

        #print("avgs:", avg)
        print("Threads, Speedup(C), Speedup(N), Throughput, Stdev")

        ########## Sequential Code ##########
        throughput = "{:.5f}".format(days[j]/avg[0])

        #variance = "{:.5f}".format(avg[0]*datasetEdges[i])
        #stdev = statistics.stdev()

        print("Thread 0   :  0.00000  , 0.00000  , ",
              throughput, " ,", "{:.5f}".format(stddev[0]))
        ########## End Sequential Code ##########

        ########## For all Confs ##########
        for y in range(1, len(avg)):

            speedupClassic = "{:.5f}".format(avg[0]/avg[y])
            speedupN = "{:.5f}".format(avg[y-1]/avg[y])
            throughput = "{:.5f}".format(days[j]/avg[y])
            sdv = "{:.5f}".format(stddev[y])
            #variance = "{:.5f}".format(avg[y]*datasetEdges[i])

            print("Thread", threads[y], "  : ",
                  speedupClassic, " ,", speedupN, " ,", throughput, " ,", sdv)
        ########## End all Confs ##########


print("Total Runs:", len(datasets) * len(days) * len(threads) * tests)

print("Cleaning...")
make = subprocess.getoutput("make clean")
print(make)
