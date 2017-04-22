import sys
import re

if len(sys.argv) != 4:
    print("Requires three arguments: read-file-log.txt voltages-filename.csv temps-filename.csv")
    sys.exit(1)

read_file = sys.argv[1]
voltages_file = sys.argv[2]
temps_file = sys.argv[3]

with open(read_file, 'r') as f:
    lines = f.readlines()

def get_voltages(parts):
    vals = []
    for part in parts[3:]:
        vals.append(int(part.strip())/1000.0)
    return vals

def get_temps(parts):
    vals = []
    for part in parts[3:]:
        vals.append(int(part.strip())/100.0)
    return vals

def get_module(parts):
    module = re.sub(r'[A-Za-z]*', '', parts[2])
    try:
        return int(module)
    except:
        print("Could not parse", module)
        return None

def get_time(parts):
    time = re.sub(r'[A-Za-z]*', '', parts[0])
    try:
        return int(time)
    except:
        print("Could not parse", time)
        return None

voltage_lines = []
temp_lines = []

for line in lines:
    line = line.strip()
    line = re.sub(r'> ', '', line)
    line = re.sub(r'\x1B\[[A-Za-z0-9][A-Za-z0-9](D)?', '', line)
    line = re.sub(r'\x1B\[[A-Za-z]*', '', line)
    line = line.strip()
    parts = line.split(",")
    if len(parts) < 15: continue
    time = get_time(parts)
    module = get_module(parts)
    if parts[1] == 'cvs':
        voltages = get_voltages(parts)
        line = str(time) + "," + str(module) + "," + ",".join(map(str, voltages))
        voltage_lines.append(line)
    elif parts[1] == 'temps':
        temps = get_temps(parts)
        line = str(time) + "," + str(module) + "," + ",".join(map(str, temps))
        temp_lines.append(line)

print(temp_lines)
print(voltage_lines)
with open(temps_file, 'w') as f:
    print("Writing temperatures to file",temps_file)
    for line in temp_lines:
        f.write(line + "\n")
with open(voltages_file, 'w') as f:
    print("Writing voltages to file",voltages_file)
    for line in voltage_lines:
        f.write(line + "\n")
