#!/usr/bin/env python
# coding: utf-8

import numpy as np
import matplotlib.pyplot as plt
import math
plt.rcParams['figure.dpi'] = 130 # Adjusting figure size

proc = [16, 36, 49, 64]
N = [2**i for i in range(4, 11)]
part = ["Multiple sends/receives", "MPI pack/unpack", "Derived datatypes"]

for p in proc:
    opt1 = [[] for i in range(len(N))]
    opt2 = [[] for i in range(len(N))]
    opt3 = [[] for i in range(len(N))]
    
    data_file = "data"+str(p)+".txt"
    data = open(data_file, "r")
    
    for line in data:
        line = line.strip().split(' ')
        opt = int(line[0].split(':')[-1])
        n_inx = N.index(int(line[2].split(':')[-1]))
        time = float(line[-1].split(':')[-1])
        
        if opt == 1:
            opt1[n_inx].append(time)
        elif opt == 2:
            opt2[n_inx].append(time)
        elif opt == 3:
            opt3[n_inx].append(time)
    
    colors  = ['r','y','m']
    name = "plot"+str(p)+".png"
    #plt.figure(figsize=(12,8), dpi=80)
    plt.figure()
    plt.plot(range(1,len(N)+1), np.array([np.median(times) for times in opt1]),color = colors[0], label=part[0])
    plt.boxplot(opt1, labels = N, medianprops = {'color':colors[0]}, boxprops = dict(color=colors[0]))
    
    plt.plot(range(1,len(N)+1), np.array([np.median(times) for times in opt2]),color = colors[1], label=part[1])
    plt.boxplot(opt2, labels = N, medianprops = {'color':colors[1]}, boxprops = dict(color=colors[1]))
    
    plt.plot(range(1,len(N)+1), np.array([np.median(times) for times in opt3]),color = colors[2], label=part[2])
    plt.boxplot(opt3, labels = N, medianprops = {'color':colors[2]}, boxprops = dict(color=colors[2]))
    
    plt.legend()
    plt.yscale('log')
    plt.xlabel('Datasize (N)')
    plt.title("Comparison of different halo exhange methods for P={}".format(p))
    plt.ylabel('Time taken (in seconds)')
    plt.savefig(name)

