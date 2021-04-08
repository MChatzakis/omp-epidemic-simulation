import subprocess

datasets = ["../datasets/facebook.txt",
            "../datasets/enron.txt", "../datasets/gnutella.txt"]
seeds = ["../datasets/fbSeed.txt",
         "../datasets/enronSeed.txt", "../datasets/gnuSeed.txt"]

#output = subprocess.getoutput(
 #   "gcc ../source/Epidemic.c -lm -fopenmp -o epidemic")

#output = subprocess.getoutput(
#    "./epidemic -f " + datasets[0] + " -s " + seeds[0])

s = "HAHA\nLOLO"
s = s.split('\n',0)[-1]
print()