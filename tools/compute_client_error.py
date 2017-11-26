#!/usr/bin/env python

import math
import sys

def usage():
    print "Usage:"
    print "compute_client_error.py server-data[,time,x,z] client-data[time,x,z]"
    print "The files are expected to contain space separated fields. The"
    print "option time,x,z specification is the field number of the STK world"
    print "time, x and z coordinate."
    print "The location of the kart in each field are specified by the"
    print "field attribute. It is the field number (space separated fields)"
    print "of the x coordinate, with the z coordinate being field+2"
    print "It computes for each data point in the client file the closest"
    print "the two data points with the closest time stamp in the server"
    print "and then interpolates the server position based on the client"
    print "time between these positions. The difference between the"
    print "intepolated position"

    sys.exit()

# -----------------------------------------------------------------------------
def splitArg(s):
    l = s.split()
    if len(l)==1:
        return s,5,8,10   # Default for current debug output
    if len(l)!=4:
        print "Incorrect specification: '%s'"%s
        usage()
    return l[0], int(l[1]), int(l[2]), int(l[3])
# -----------------------------------------------------------------------------
def readFile(name, time, x, z):
    f = open(name, "r")
    result = []
    for i in f.readlines():
        if i[:6] == "Rewind":
            continue
        l = i.split()
        result.append( (float(l[time]),
                        float(l[x]   ),
                        float(l[z]   ) ) )
    f.close()
    return result
        
# -----------------------------------------------------------------------------
# Compares the client and server data. Each argument is a list of
# triplets containing (time, x, z) data.
#
def computeDifferences(server, client):
    index_s = 0

    count = 0
    sum   = 0.0
    min   = 999999.9
    max   = -1.0
    for (time, x, z) in client:
        # Find largest entry in server data that is <= client's time:
        while index_s<len(server)-2:
            t1 = server[index_s+1][0]
            if t1>time: break
            index_s += 1

        #print "time", time, server[index_s][0], server[index_s+1][0]

        t1,x1,z1 = server[index_s  ]
        t2,x2,z2 = server[index_s+1]
        f = (time-t1)/(t2-t1)
        x_i = x1 + f * (x2-x1)
        z_i = z1 + f * (z2-z1)
        error = math.sqrt((x-x_i)**2 + (z-z_i)**2)
        #print t1,x1,t2,x2,time,x_i
        print time, error
        if (error < min): min=error
        if (error > max): max=error
        count += 1
        sum += error

    print "sum %f count %d min %f average %f max %f" \
        % (sum, count, min, sum/count, max)
# -----------------------------------------------------------------------------

if __name__=="__main__":
    if len(sys.argv)!=3:
        usage()
    server_name, time_s, x_s, z_s = splitArg(sys.argv[1])
    client_name, time_c, x_c, z_c = splitArg(sys.argv[2])

    server = readFile(server_name, time_s, x_s, z_s)
    client = readFile(client_name, time_c, x_c, z_c)
    
    computeDifferences(server, client)
