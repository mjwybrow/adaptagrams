#!/usr/bin/python

import os

def genNM():
    nm = []
    for n in range(10, 210, 10):
        M = int(round(1.5*n))
        d = int(round((M-n)/5))
        nm.extend([(n,m) for m in range(n, M+1, d)])
    return nm

if __name__ == '__main__':
    nm = genNM()
    for n, m in nm:
        os.system('./randomGraph -a %d %d' % (n, m))
