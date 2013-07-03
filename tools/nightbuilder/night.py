#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)
################################################################################

# Script used to build nighlies



import os
import sys
import getopt

# import the configuration file
from config import *

# import functions/class 
from utils import *
from svn import *
from build import *
from package import *
from send import *

def main():
    # error 
    noBuildErr = False
    nosvnErr = False
    isBuilt = False
    # parse input FIXME The parser doesn't work
    #parser(sys.argv[1:])
    # welcome message
    print
    print "nightly builder for supertuxkart"
    print "Copyright (C) 2012 Jean-manuel clemencon (samuncle)"
    print separator(COLOR.OKBLUE)
    
    # display help
    if (ARG["HELP"]):
        usage()
        print separator(COLOR.OKBLUE)
        print
        exit()
    
    # svn part -----------------------------------------------------------------
    # init the svn
    mysvn = Svn(CONFIG["WORKINGDIR"])
    print "current svn revision: " + str(mysvn.getLocalRevision())
    # Update the svn
    if (ARG["UPDATE"]):
        space = bufferedOutput("start svn update @ " + getTime())
        try:
            mysvn.update()
            nosvnErr = True
            sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
        except:
            sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")
        
        # If no error occured
        if (nosvnErr):
            print "svn updated rev: " + str(mysvn.getLastRevision())
    
    # buid part ----------------------------------------------------------------
    # init the build
    mybuild = Build(CONFIG["WORKINGDIR"] + "/cmake_build")
    # Clean the project
    if (ARG["CLEAN"]):
        space = bufferedOutput("start clean @ " + getTime())
        try:
            mybuild.clean()
            sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
        except:
            sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")
    
    # build the project (only if no error and the revision has changed
    if (nosvnErr):
        if (mysvn.getIsChanged()):
            print "revision changed"
            space = bufferedOutput("start compilation @ " + getTime())
            mybuild.make(ARG["JOB"])
            isBuilt = True
            if (mybuild.noError()):
                noBuildErr = True
                sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
            else:
                sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")
        else:
            print "revsion not changed"
    
    # Build the project (force)
    if (ARG["FORCE"] and (not isBuilt) ):
        space = bufferedOutput("start forced compilation @ " + getTime())
        mybuild.make(ARG["JOB"])
        if (mybuild.noError()):
            noBuildErr = True
            sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
        else:
            sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")
    
    # package part -------------------------------------------------------------
    mypack = Package(mysvn.getLastRevision(), CONFIG["COMPRESSDIR"],CONFIG["OS"],CONFIG["PLATFORM"])
    # pack the binary
    if (noBuildErr and ARG["BIN"]):
        space = bufferedOutput("start bin file compress @ " + getTime())
        try:
            mypack.addFile("supertuxkart", "stkbin", CONFIG["BUILDDIR"] + "/bin")
            sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
        except:
            sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")
    
    # pack the data
    if(ARG["DATA"]):
        space = bufferedOutput("start data file compress @ " + getTime())
        try:
            mypack.addDir("data", "stkdat", CONFIG["WORKINGDIR"], "*.svn*")
            sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
        except:
            sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")
    
    # network part -------------------------------------------------------------
    if(ARG["SEND"]):
        # send file by FTP
        space = bufferedOutput("start file(s) transfer by FTP @ " + getTime())
        myFiles = mypack.getFiles()
        mysend = Send(CONFIG["FTPHOST"],CONFIG["FTPUSER"],CONFIG["FTPPASS"],CONFIG["SCRIPTDIR"])
        for i in myFiles:
            mysend.add(i, CONFIG["COMPRESSDIR"], CONFIG["REMOTEDIR"])
        #FIXME The ftp didn't throw an exception.
        try:
            mysend.send()
            sys.stdout.write(COLOR.OKGREEN + space + "[DONE]" + COLOR.ENDC + "\n")
        except:
            sys.stdout.write(COLOR.WARNING + 74 * " " + "[FAIL]" + COLOR.ENDC + "\n")

    print separator(COLOR.OKBLUE)
    print

if __name__ == "__main__":
    main()
    
    #sys.argv[1:]
