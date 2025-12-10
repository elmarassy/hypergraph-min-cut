import matplotlib.pyplot as plt
import numpy as np

def exp2_generate_time_comp():
    deterministic = open("./outputs/exp2Deterministic.txt", "r")
    randomized = open("./outputs/exp2Randomized.txt", "r")
    plot_names = ["c17", "c432", "c499", "c880", "c1355", "c1908", "c2670", "c3540", "c5315", "c6288", "c7552"]

    deterministic_times = [0]*len(plot_names)
    randomized_times = [0]*len(plot_names)
    
    for line in deterministic.readlines():
        split_line = line.split(" ")
        name = split_line[0].split(".")[0]
        idx = plot_names.index(name)
        deterministic_times[idx] = int(split_line[3])/(10**6)
    
    for line in randomized.readlines():
        split_line = line.split(" ")
        name = split_line[0].split(".")[0]
        idx = plot_names.index(name)
        randomized_times[idx] = int(split_line[3])/(10**6)

    bar_width = 0.35
    x = np.arange(len(plot_names))
    plt.bar(x - bar_width/2, deterministic_times, bar_width, label='KW Times')
    plt.bar(x + bar_width/2, randomized_times, bar_width, label='FPZ Times')
    
    plt.xlabel('Circuit Graphs')
    plt.ylabel('Time (s)')
    plt.title('Time of Running Circuit Hypergraphs on KW vs. FPZ')
    plt.xticks(x, plot_names)
    plt.legend()
    plt.tight_layout()
    plt.show()
    


