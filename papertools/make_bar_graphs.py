import csv
import sys

INITIAL_STRING = '''\\begin{tikzpicture}
\\begin{axis}[
  ybar stacked,
  symbolic x coords={A, B, C, D, E, F, G},
]'''

END_STRING = '''\end{axis}
\end{tikzpicture}'''

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

columns = [6,7,8,9,10,11,12]

print(INITIAL_STRING)
for col in columns:
    line = '\\addplot coordinates {'
    counter = 0
    for row in dataset:
        line += '('
        line += chr(ord('A') + counter)
        line += ','
        line += str(row[col] / 1000)
        line += ') '
        counter += 1
    line += '};'
    print(line)
line = '\\legend{'
first = True
for col in columns:
    if first:
        first = False
    else:
        line += ','
    line += column_names[col]
line += '}'
print(line)
print(END_STRING)
