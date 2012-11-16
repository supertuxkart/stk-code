#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)
################################################################################

import os
import sys
from time import gmtime, strftime

# import config
from config import *


class Cdir:
    """
    A class used to change the directory and reset it when it's destructed
    """

#-------------------------------------------------------------------------------
    def __init__ (self, path):
        self.oriPath = os.getcwd()
        os.chdir(path)

#-------------------------------------------------------------------------------
    def __del__ (self):
        os.chdir(self.oriPath)

class COLOR:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

def separator(color):
    return color + 80 * "-" + COLOR.ENDC

#-------------------------------------------------------------------------------
# usage of the script. Displayed if -h is invoqued
def usage(error = ""):
    if (error):
        print "[error] " + error
    h = [
    " Options avaliables:",
    "    --bin         # package the binary",
    "    --data        # package the data",
    "    --clean       # remove all packages and logs",
    "    --send        # send the package via FTP",
    "    --force       # force the build (even the revision hasn't changed)",
    "    --update      # update the SVN",
    "    --web         # html output"
    "    --job=        # like -j for make",
    "    --help        # display help",
    ]
    for i in h:
        print i 

def getTime():
    return strftime("%a, %d %b %Y %H:%M:%S GMT+01", gmtime())

#-------------------------------------------------------------------------------
# Used to format output
def bufferedOutput(string, nb = 74):
    space = (nb - len(string)) * " "
    sys.stdout.write(string)
    sys.stdout.flush()
    return space

#-------------------------------------------------------------------------------
def parser(argv):
    a = os.system("ls")
    print a
    try:
        opts, args = getopt.getopt(argv, "bdcsfuhj:", ["bin",
                                                     "data", 
                                                     "clean",
                                                     "send",
                                                     "force",
                                                     "update",
                                                     "help",
                                                     "job="
                                                     ])
        for opt, args in opts:
            if opt in ("-h", "--help"):
                ARG["HELP"] = True
                """
            if opt in ("-b", "bin"):
                ARG["BIN"] = True
            if opt in ("-d", "data"):
                ARG["DATA"] = True
            if opt in ("-s", "send"):
                ARG["SEND"] = True
            if opt in ("-f", "force"):
                ARG["FORCE"] = True
            if opt in ("-u", "update"):
                ARG["UPDATE"] = True
                """
    except:
        usage("unrecognized option")
