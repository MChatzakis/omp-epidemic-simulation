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
threads = [1, 2, 4]
datasets = ["../datasets/facebook.txt",
            "../datasets/gnutella.txt", "../datasets/enron.txt"]
seeds = ["../datasets/fbSeed.txt",
         "../datasets/gnuSeed.txt", "../datasets/enronSeed.txt"]

avg = []

for i in range(len(datasets)):
    print("---------------------------------------------------")
    print(bcolors.OKBLUE, "Dataset", "\"",datasets[i],"\"", bcolors.ENDC)
    for j in range(len(days)):
        print("Days:", days[j])
        for k in range(len(threads)):
            #print("Calculating average time for ", threads[k], "threads")
            for m in range(tests):
                command = "./epidemic -f " + \
                    datasets[i] + " -s " + seeds[i] + " -d " + \
                    str(days[j]) + " -t " + str(threads[k]) + " -c"
                # print(command)
                out = subprocess.getoutput(command)
                # print(out)
            # read file
            values = []
            with open('timeCalculations.txt', 'r+') as f:
                for line in f:
                    if line:  # avoid blank lines
                        values.append(float(line.strip()))
            avrg = "{:.5f}".format(sum(values) / len(values))
            print("--Thread(s) #",threads[k], values, "=> avg: ", avrg,"s")
            # print(bcolors.OKGREEN, "Average time for ", threads[k], "thread(s) is", , bcolors.ENDC)
            os.remove("timeCalculations.txt")
    #print("---------------------------------------------------")


print("Total Runs:", len(datasets) * len(days) *len(times))    
