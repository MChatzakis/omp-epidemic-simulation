import subprocess
import re

datasets = ["../datasets/facebook.txt",
            "../datasets/enron.txt", "../datasets/gnutella.txt"]
seeds = ["../datasets/fbSeed.txt",
         "../datasets/enronSeed.txt", "../datasets/gnuSeed.txt"]
         
with open("myfile") as f:
    floats = map(float, f)
