import matplotlib.pyplot as plt
import numpy as np

def exp2_generate_time_comp():
    deterministic = open("./outputs/exp2Deterministic.txt", "r")
    randomized = open("./outputs/exp2Randomized.txt", "r")
    plot_names = ["c17", "c432", "c499", "c880", "c1355", "c1908", "c2670", "c3540", "c5315", "c6288", "c7552"]

    deterministic_times = [10]*len(plot_names)
    randomized_times = [10]*len(plot_names)
    
    hyperff_sizes = [0]*10
    deterministic_hyperff_times = [0]*10
    randomized_hyperff_times = [0]*10
    
    next_type = False
    counter = 0
    
    for line in deterministic.readlines():
        if line[0:9] == "file_name":
            continue
        if line == "\n":
            next_type = True
            continue
        if not next_type:
            split_line = line.split(" ")
            name = split_line[0].split(".")[0]
            idx = plot_names.index(name)
            deterministic_times[idx] = int(split_line[-1])/(10**6)
        else:
            split_line = line.split(" ")
            name = split_line[0].split(".")[0]
            size = int(name[5])
            deterministic_hyperff_times[size] = int(split_line[-1])/(10**6)
            hyperff_sizes[size] = 500+size*100
    
    next_type = False
    for line in randomized.readlines():
        if line[0:9] == "file_name":
            continue
        if line == "\n":
            next_type = True
            continue
        if not next_type:
            split_line = line.split(" ")
            name = split_line[0].split(".")[0]
            idx = plot_names.index(name)
            randomized_times[idx] = int(split_line[-1])/(10**6)
        else:
            split_line = line.split(" ")
            name = split_line[0].split(".")[0]
            size = int(name[5])
            randomized_hyperff_times[size] = int(split_line[-1])/(10**6)

    bar_width = 0.35
    x = np.arange(len(plot_names))
    
    fig, axs = plt.subplots(nrows=1, ncols=2, figsize=(10, 4))

    axs[0].bar(x - bar_width/2, deterministic_times, bar_width, label='KW Times')
    axs[0].bar(x + bar_width/2, randomized_times, bar_width, label='FPZ Times')
    
    axs[0].set_xlabel('Circuit Graphs')
    axs[0].set_ylabel('Time (s)')
    axs[0].set_title('Time of Running Circuit Hypergraphs on KW vs. FPZ')
    axs[0].set_xticks(x, plot_names)
    axs[0].set_yscale('log')
    axs[0].legend()
    
    x = np.arange(len(hyperff_sizes))
    
    axs[1].bar(x - bar_width/2, deterministic_hyperff_times, bar_width, label='KW Times')
    axs[1].bar(x + bar_width/2, randomized_hyperff_times, bar_width, label='FPZ Times')
    
    axs[1].set_xlabel('Number of Nodes w/ (p = 0.5, q = 0.2)')
    axs[1].set_ylabel('Time (s)')
    axs[1].set_title('Time of Running HyperFF Hypergraphs on KW vs. FPZ')
    axs[1].set_xticks(x, hyperff_sizes)
    axs[1].set_yscale('log')
    axs[1].legend()
    
    plt.tight_layout()
    plt.show()  
    
def exp1_generate_plots():
    data = open("./outputs/exp1Randomized.txt", "r")
    fig, axs = plt.subplots(nrows=1, ncols=2, figsize=(10, 4), subplot_kw={'projection': '3d'})
    
    next_graph = False
    x = np.linspace(100, 1000, num=10)
    y = np.linspace(100, 1000, num=10)
    X, Y = np.meshgrid(x, y)
    Z1 = np.zeros(X.shape)
    Z2 = np.zeros(X.shape)
    counter = 0
    
    for line in data.readlines():
        if line[0] == "n":
            continue
        if len(line) == 1:
            next_graph = True
            counter = 0
            continue
        if next_graph:
            split_line = np.array(line.split(" "))
            print(split_line)
            n, m, k = split_line[0:3].astype(int)
            units = int(split_line[-1][:-2])
            print(units)
            Z2[counter//10][counter%10] = units
        else:
            split_line = np.array(line.split(" "))
            n, m, k = split_line[0:3].astype(int)
            units = int(split_line[-1][:-2])
            print(units)
            Z1[counter//10][counter%10] = units
        counter += 1

    axs[1].set_title('k = ln n Runtimes')
    axs[1].set_xlabel('Number of Nodes')
    axs[1].set_ylabel('Number of Edges')
    axs[1].set_zlabel('Total Time (units)')

    axs[0].set_title('k = 10 Runtimes')
    axs[0].set_xlabel('Number of Nodes')
    axs[0].set_ylabel('Number of Edges')
    axs[0].set_zlabel('Total Time (units)')
    
    axs[0].plot_surface(X, Y, Z1)
    axs[1].plot_surface(X, Y, Z2)
    
    plt.tight_layout()
    plt.show()

# exp1_generate_plots()
exp2_generate_time_comp()

# 3D (n, m) graph for randomized to show asymptotic bounds. Two graphs, k = 10, k = ln n. Do same for deterministic but plot time, not units. That plot will also have randomized and deterministic.
# exp2_time_comp(): Two comparison graphs, one for circuits and one for hyperff
# 


# In future: Go for m = n^2, try on many seeds and plot