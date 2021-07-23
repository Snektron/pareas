import csv
import sys

dataset = []

INITIAL_STRING = '''\\begin{tikzpicture}
\\begin{axis}[
  ylabel=Runtime (ms)
  xlabel=Number of nodes,
]'''

END_STRING = '''\end{axis}
\end{tikzpicture}'''

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

print(INITIAL_STRING)
print('\\addplot[color=red,mark=x] coordinates {')

xcoord_col = 31
ycoord_col = 9

for row in dataset:
    line = '    ('
    line += str(row[xcoord_col])
    line += ','
    line += str(row[ycoord_col] / 1000)
    line += ')'
    print(line)
print('};')
print(END_STRING)
