import subprocess
import re
import statistics
import os


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


tests = 5
days = [30, 180, 365]
threads = [0, 1, 2, 4]
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
            avg.append(float(avrg))
            print("--Thread(s) #", threads[k], values, "=> avg: ", avrg, "s")

            os.remove("timeCalculations.txt")

            if threads[k] == 0:
                make = subprocess.getoutput("make")
                # print(make)

        #print("avgs:", avg)
        print("Threads, Speedup (classic), Speedup (n, n-1), throughput")
        throughput = "{:.5f}".format(days[j]/avg[0])
        print("Thread 0   :  0.00000  , 0.00000  , ",throughput)
        for y in range(1, len(avg)):
            speedupClassic = "{:.5f}".format(avg[0]/avg[y])
            speedupN = "{:.5f}".format(avg[y-1]/avg[y])
            throughput = "{:.5f}".format(days[j]/avg[y])
            print("Thread", threads[y], "  : ", speedupClassic, " ,", speedupN, " ,", throughput)


print("Total Runs:", len(datasets) * len(days) * tests)
