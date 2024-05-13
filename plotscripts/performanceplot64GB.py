import os
import pandas as d
import matplotlib.pyplot as plt

# Reads the data from a file
def read_data(path):
    # Reads the data from the specified file 
    data = d.read_csv(path, delimiter='\s+')
    return data

# Plot bar graphs for throughput, time, and data transfer rate against thread types
def plot_bargraphs(data):
    thread_types = ['HashThreads', 'SortThreads', 'WriteThreads']
    metrics = ['PerformanceTime(sec)']
    
    # Create a directory to save the images if it doesn't exist
    output_dir = 'graphs'
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    for metric in thread_types:
        fig, axs = plt.subplots(1, 3, figsize=(18, 6))
        for i, thread_type in enumerate(metrics):
            ax = axs[i]
            ax.bar(data[thread_type], data[metric])
            ax.set_xlabel(metric)
            ax.set_ylabel(thread_type)
            ax.set_title(f'{metric} vs {thread_type}')

        plt.tight_layout()
        # Save the image with a descriptive name
        image_path = os.path.join(output_dir, f'64GB_{metric.replace("/", "-")}_bargraph.png')
        plt.savefig(image_path)
        plt.show()

def main():
    # Input file path
    path = os.path.join('../input', 'configurations_time_64gb.txt')
    # Read the data
    data = read_data(path)

    # Plot bar graphs for throughput, time, and data transfer rate
    plot_bargraphs(data)

if __name__ == "__main__":
    main()
