import re
import sys

fname = sys.argv[1]
f = open(f"{fname}", "r")
txt = f.read()
f.close()
edit = re.findall(r'\.(\w+)s', txt)
times = []
print(edit)

for x in range(len(edit)):
        times.append(float(edit[x].strip("ms")))

i=0
while i<len(times):
    sum=0
    for k in range(20):
        sum+=times[i]
        i+=1
    avg=sum/20
    print(avg)
