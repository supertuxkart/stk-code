#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)
# Class for the SVN
################################################################################

import os
from subprocess import check_output 
from utils import *

class Svn:
    """
    Interface for the SVN
    """
    
    __svnInfoLocal = ""     # info of the local repo
    __svnInfoLast  = ""     # info when it's updated
    __localRev     = 0      # the local revision number
    __lastRev      = 0      # the updated revision number
    __isUp         = False  # is the repo has been already updated

#-------------------------------------------------------------------------------
    def __init__ (self, workingDir):
        """
        Constructor of the svn class
        """
        # local copy for the configuration
        self.__workingDir = workingDir
        
        # cd to the working dir and get info from current revision
        changeDir = Cdir(self.__workingDir)
        self.__svnInfoLocal = check_output(["svn info"], shell=True)
        del changeDir
        
        # exctract the local revision
        self.__localRev = self.__parseInfo(self.__svnInfoLocal)

#-------------------------------------------------------------------------------
    def __parseInfo(self, strIn):
        """
        Parse info to extract the revision
        """
        workingData = strIn.split("\n")
        for i in workingData:
            if "Revision: " in i:
                return int(i.split("Revision: ")[1])
        
#-------------------------------------------------------------------------------
    def update(self):
        """
        update the repository (svn up)
        """
        # cd to the directory and update the svn
        changeDir = Cdir(self.__workingDir)
        self.__svnInfoLocal = check_output(["svn up"], shell=True)
        self.__svnInfoLast = check_output(["svn info"], shell=True)
        del changeDir
        
        # exctract the last revision
        self.__lastRev = self.__parseInfo(self.__svnInfoLast)
        
        # now we have updated the SVN
        self.__isUp = True

#-------------------------------------------------------------------------------
    def getLocalRevision(self):
        """
        return the local revision 
        """
        return self.__localRev
        
#-------------------------------------------------------------------------------
    def getLastRevision(self):
        """
        return the last revision
        """
        if not (self.__isUp): 
            raise Exception ("The revision has not been updated.")
        return self.__lastRev

#-------------------------------------------------------------------------------
    def getIsChanged(self):
        """
        return true if revision has changed
        """
        if not (self.__isUp): 
            raise Exception ("The revision has not been updated.")
            
        if (self.__lastRev != self.__localRev):
            return True
        else:
            return False

