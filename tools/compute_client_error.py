#!/usr/bin/env python

import math
import sys

def usage():
    print("Usage:")
    print("compute_client_error.py -f time,x1[,x2,x3....] server-data client-data")
    print("The files are expected to contain space separated fields. The")
    print("-f options specifies first the column in which the world time")
    print("is, followed by the list of columns to be compared.")
    print("It computes for each data point in the client file the closest")
    print("the two data points with the closest time stamp in the server")
    print("and then interpolates the server position based on the client")
    print("time between these positions. The difference between the")
    print("intepolated position")
    print()
    print("Example:")
    print("compute_client_error.py-multi -f 6,16,17,18 debug.server debug.client")
    print("   to compute the differences between client and server for the")
    print("   fields 16,17,18 (which atm is the velocity)")
    sys.exit()

# -----------------------------------------------------------------------------
def readFile(name, fields):
    f = open(name, "r")
    result = []
    for i in f.readlines():
        if i[:6] == "Rewind":
            continue
        l = i.split()
        try:
            l_values = [ float(l[index]) for index in fields ]
        except ValueError:
            pass
        result.append(l_values)
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
    for items in client:
        time, x = items[0], items[1:]
        # Find largest entry in server data that is <= client's time:
        while index_s<len(server)-2:
            t1 = server[index_s+1][0]
            if t1>time: break
            index_s += 1

        #print "time", time, server[index_s][0], server[index_s+1][0]

        t1,x1 = server[index_s  ][0],server[index_s  ][1:]
        t2,x2 = server[index_s+1][0],server[index_s+1][1:]
        f = (time-t1)/(t2-t1)
        interp = []
        error  = 0
        for i, x1_val in enumerate(x1):
            x2_val = x2[i]
            x_i = x1_val + f * (x2_val-x1_val)
            interp.append(x_i)
            error = error + (x[i]-x_i)**2
        error = math.sqrt(error)
        print(time, error)
        if (error < min): min=error
        if (error > max): max=error
        count += 1
        sum += error

    print("sum %f count %d min %f average %f max %f" \
        % (sum, count, min, sum/count, max))
# -----------------------------------------------------------------------------

if __name__=="__main__":
    if len(sys.argv)==5 and sys.argv[1]=="-f":
        fields = sys.argv[2]
        del sys.argv[1:3]
    else:
        fields = ["6", "9", "11"]
    
    if len(sys.argv)!=3:
        usage()

    # -1 to convert awk/gnuplot indices (starting with 1) to
    # python indices (starting with 0)
    field_indices = [int(item)-1 for item in fields.split(",")]
    
    server_name = sys.argv[1]
    client_name = sys.argv[2]

    server = readFile(server_name, field_indices)
    client = readFile(client_name, field_indices)
    
    computeDifferences(server, client)
