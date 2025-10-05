import os
from time import sleep

parentPid = os.getpid()

outputDir = "/home/user/Desktop/testOutput"

childPids = []
maxPids = 5

print("Parent PID:", parentPid)
for idx in range(0, maxPids):
    currentPid = os.fork()

    if currentPid < 0: # failed child
        print(f"Child {idx} failed to fork...")
    
    if currentPid == 0: # child process
        print(f"child {idx} is active...")
        sleep(1+idx)
        os._exit(0)
    else: # parent process
        childPids.append(currentPid)

for i in range(0, maxPids):
    status = os.waitpid(childPids[i], 0)
    print(f"Process {childPids[i]} exited with status {status[1]}.")
