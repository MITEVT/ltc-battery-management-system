import sys
import re

if len(sys.argv) != 5:
    print("Requires four arguments: read-file-log.txt voltages-filename.csv temps-filename.csv pack_current-filename.csv")
    sys.exit(1)

read_file = sys.argv[1]
voltages_file = sys.argv[2]
temps_file = sys.argv[3]
pack_current_file = sys.argv[4]

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

def get_pack_current(parts):
    return parts[2]

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
pack_current_lines = []

for line in lines:
    line = line.strip()
    line = re.sub(r'> ', '', line)
    line = re.sub(r'\x1B\[[A-Za-z0-9][A-Za-z0-9](D)?', '', line)
    line = re.sub(r'\x1B\[[A-Za-z]*', '', line)
    line = line.strip()
    parts = line.split(",")
    if len(parts) < 3: continue
    if (not (parts[1]=='csv' or parts[1]=='temps' or parts[1]=='pcurr')): continue
    time = get_time(parts)
    if parts[1] == 'cvs':
        module = get_module(parts)
        voltages = get_voltages(parts)
        line = str(time) + "," + str(module) + "," + ",".join(map(str, voltages))
        voltage_lines.append(line)
    elif parts[1] == 'temps':
        module = get_module(parts)
        temps = get_temps(parts)
        line = str(time) + "," + str(module) + "," + ",".join(map(str, temps))
        temp_lines.append(line)
    elif parts[1] == 'pcurr':
        pack_current = get_pack_current(parts);
        line = str(time) + "," + str(pack_current)
        pack_current_lines.append(line)


print(temp_lines)
print(voltage_lines)
print(pack_current_lines)
with open(temps_file, 'w') as f:
    print("Writing temperatures to file",temps_file)
    for line in temp_lines:
        f.write(line + "\n")
with open(voltages_file, 'w') as f:
    print("Writing voltages to file",voltages_file)
    for line in voltage_lines:
        f.write(line + "\n")
with open(pack_current_file, 'w') as f:
    print("Writing pack current to file", pack_current_file)
    for line in pack_current_lines:
        f.write(line + "\n")
