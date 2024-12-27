import csv
import re

def parse_file(file_path):
    data = {}
    current_dataset = None
    current_algorithm = None
    current_size = None

    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()

            if line.startswith('dataset:'):
                current_dataset = line.split(':')[1].strip()
            elif line.startswith('algorithm:'):
                current_algorithm = line.split(':')[1].strip()
            elif line.startswith('size:'):
                current_size = int(line.split(':')[1].strip())
            elif line.startswith('|'):
                fields = line.split('|')[1:]
                if len(fields) >= 5:
                    try:
                        ns_op = float(fields[0].strip())
                        key_gen_type = fields[4].strip()

                    except:
                        continue

                    if current_dataset not in data:
                        data[current_dataset] = {}
                    if current_algorithm not in data[current_dataset]:
                        data[current_dataset][current_algorithm] = {
                            'size': current_size
                        }

                    if "chained" in key_gen_type:
                        data[current_dataset][current_algorithm]['ns_op_chained'] = ns_op
                    elif "independent" in key_gen_type:
                        data[current_dataset][current_algorithm]['ns_op_independent'] = ns_op


    return data


def generate_csv_tables(data, output_prefix):
    datasets = list(data.keys())
    algorithms = sorted(set(algo for dataset in data.values() for algo in dataset.keys()))

    # Table 1: Chained Algorithm Performance (ns/op) vs Dataset
    with open(f'{output_prefix}_chained_performance.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Dataset'] + algorithms)
        for dataset in datasets:
            row = [dataset] + [data[dataset].get(algo, {}).get('ns_op_chained', '') for algo in algorithms]
            writer.writerow(row)

    # Table 2: Independent Algorithm Performance (ns/op) vs Dataset
    with open(f'{output_prefix}_independent_performance.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Dataset'] + algorithms)
        for dataset in datasets:
            row = [dataset] + [data[dataset].get(algo, {}).get('ns_op_independent', '') for algo in algorithms]
            writer.writerow(row)

    # Table 3: Algorithm Size (bytes) vs Dataset
    with open(f'{output_prefix}_algorithm_size.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Dataset'] + algorithms)
        for dataset in datasets:
            row = [dataset] + [data[dataset].get(algo, {}).get('size', '') for algo in algorithms]
            writer.writerow(row)


# Usage example
file_path = 'bench_output2.txt'
output_prefix = 'output'

data = parse_file(file_path)
generate_csv_tables(data, output_prefix)
