#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)
# Class used to build the project
################################################################################
from subprocess import check_output
from utils import *

class Build:
    """
    Interface for the builder
    """
    
    # if an error occured
    __noError = True

#-------------------------------------------------------------------------------
    def __init__ (self, buildDir):
        """
        Constructor of the builder class
        """
        self.__buildDir = buildDir

#-------------------------------------------------------------------------------
    def make(self, job):
        """
        the make command
        """
        changeDir = Cdir(self.__buildDir)

        # we try to build supertuxkart
        try:
            check_output(["make -j" + str(job)], shell=True)
        except:
            self.__noError = False
        del changeDir

#-------------------------------------------------------------------------------
    def clean(self):
        """
        the clean command
        """
        changeDir = Cdir(self.__buildDir)
        check_output(["make clean"], shell=True)
        del changeDir

#-------------------------------------------------------------------------------
    def noError(self):
        """
        return true if no error
        """
        return self.__noError
