#!/usr/bin/env python
# coding: utf-8

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import math
plt.rcParams['figure.dpi'] = 130 # Adjusting figure size

N = [2**i for i in range(0, 3)]
part = ['P=1', 'P=2']

opt1 = [[] for i in range(len(N))]
opt2 = [[] for i in range(len(N))]

with open('data.csv', 'r') as csvfile:
    #data= csv.reader(csvfile, delimiter=',')
    for line in csvfile:
      row = line.strip().split(",")
      if int(row[0]) == 1:
        opt1[int(np.log2(int(row[1])))].append(float(row[2]))
      else:
        opt2[int(np.log2(int(row[1])))].append(float(row[2]))

colors  = ['r','m']
name = "plot.jpg"

plt.figure()
plt.plot(range(1,len(N)+1), np.array([np.median(times) for times in opt1]),color = colors[0], label=part[0])
plt.boxplot(opt1, labels = N, medianprops = {'color':colors[0]}, boxprops = dict(color=colors[0]))

plt.plot(range(1,len(N)+1), np.array([np.median(times) for times in opt2]),color = colors[1], label=part[1])
plt.boxplot(opt2, labels = N, medianprops = {'color':colors[1]}, boxprops = dict(color=colors[1]))

plt.legend()
plt.xlabel('Processes per Node(PPN)')
plt.title("Time Consumed vs PPN: Scaling Up Comparison")
plt.ylabel('Time taken (in seconds)')
plt.savefig(name)
