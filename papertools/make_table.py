import csv
import sys

dataset = []

def process(x):
    x = str(x).replace(',','.')
    
    try:
        return float(x)
    except:
        return x

found_column_names = False
with open('processed.csv', 'r') as input:
    reader = csv.reader(input)
    
    for row in reader:
        if not found_column_names:
            column_names = row
            found_column_names = True
        if row[0].startswith(sys.argv[1]):
            new_row = [process(x) for x in row]
            dataset.append(new_row)

columns = [13,6,7,8,9,10,11,12]

for row in dataset:
    first = True
    for column in columns:
        if first:
            first = False
        else:
            print(' & ', end='')
        print(round(row[column] / 1000, 3), end='')
    print(' \\\\ \\hline')
