import subprocess
datasets = ["../datasets/facebook.txt",
            "../datasets/enron.txt", "../datasets/gnutella.txt"]
seeds = ["../datasets/fbSeed.txt",
         "../datasets/enronSeed.txt", "../datasets/gnuSeed.txt"]
output = subprocess.getoutput(
    "gcc ../source/Epidemic.c -lm -fopenmp -o epidemic")

print(datasets[0])
output = subprocess.getoutput("./epidemic -f "+ datasets[0] + " -s " + seeds[0])

print(output)
