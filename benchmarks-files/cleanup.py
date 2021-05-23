# Script to parse output raw data from Ann-Benchmarks and move data and cleanup, 
# to make raedy for next run.

print("Loading file")
with open("exports/exports.csv") as f:
    contents = f.read().splitlines()

coords = {}

print("Parsing file")
count = 3
for i in range(0, len(contents)):
  line = contents[i]

  if line.startswith("\"eCP"):
    id = line.split('(')[1]

    if id in coords:
      vals = coords[id]
      coords[id] = (vals[0] + float(contents[i+2]), vals[1] + float(contents[i+1]))
    else:
      coords[id] = (float(contents[i+2]), float(contents[i+1]))

for k,v in coords.items():
  x = float(v[0])/count
  y = float(v[1])/count
  print(k + ": " + "(" + str(x) + ", " + str(y) + ")")

print("Cleanup done.")