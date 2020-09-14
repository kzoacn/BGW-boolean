from math import *


for m in range(3,6):
    for r in range(3,10):
        n=r*m
        for o in range(2,r):
            p=1-1.0*o*(o-1)/r/(r-1)
            for t in range( 5,200):
                if t*log(p)<-40*log(2.0):
                    print(m,r,o,t,'size=',o*n*t)
                    break
        
