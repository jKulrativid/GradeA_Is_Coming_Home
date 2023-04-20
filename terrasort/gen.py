#!/usr/bin/python3
import sys
import random

def pout(k , v):
    print(k+": "+str(v))


def rev(max=1999999):
    MAX=max
    for i in range(MAX,0,-1):
        k='std-'+str(MAX-i+1)
        v=float(i)
        pout(k,v)

def rand(max=1999999):
    MAX=max
    for i in range(MAX, 0, -1):
        k='std-'+str(MAX-i+1)
        v=random.random()
        pout(k,v)


def run(alg,max):
    runner = {
            "rev" : lambda max: rev(max),
            "rand" : lambda max: rand(max)
            }
    return runner.get(alg)(max)

argv=sys.argv
if len(argv) < 3:
    print("Usage: %s [rev, rand] size" % argv[0])
    exit()

alg=sys.argv[1]
max=int(sys.argv[2])

run(alg,max)
