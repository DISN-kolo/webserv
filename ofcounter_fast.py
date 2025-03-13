#!/bin/python3
import sys
import os
from time import sleep

stringToRun = "ls /proc/" + sys.argv[1] + "/fd/ | wc -l"

while (True):
    os.system(stringToRun)
    sleep(0.01)
