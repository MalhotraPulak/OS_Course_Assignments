import sys
chunk = 1000
total = int(sys.argv[2])
with open(sys.argv[1], "w") as ff:
   for i in range(total//chunk):
        ff.write(str(i % 10) * 1000);        
