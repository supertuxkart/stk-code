#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)
# Class used to package the project
################################################################################
from subprocess import check_output
from utils import *

class Package:
    """
    Interface for the builder
    """
    __revision = 0
    __listFile = []
    
#-------------------------------------------------------------------------------
    def __init__ (self, revision, compressDir, os,platform):
        """
        Constructor of the builder class
        """
        self.__os = os
        self.__revision = str(revision)
        self.__platform = platform
        self.__compressDir = compressDir

#-------------------------------------------------------------------------------
    def addFile(self, inFile, outFile, workingDir):
        """
        Packing a file
        """
        # filename
        name = outFile                      \
               + "_" + self.__os            \
               + self.__platform            \
               + "_" + self.__revision      \
               + ".zip"
        self.__listFile.append(name)
        
        # prepare the command
        command = "zip "                    \
                  + self.__compressDir      \
                  + "/" + name              \
                  + " " + inFile

        # execute the command to pack the binary
        changeDir = Cdir(workingDir)
        check_output([command], shell=True)
        del changeDir

#-------------------------------------------------------------------------------
    def addDir(self, inFile, outFile, workingDir, exclude):
        """
        Packing a directory
        """
        # filename
        name = outFile                      \
               + "_" + self.__os            \
               + "_" + self.__revision      \
               + ".zip"
        self.__listFile.append(name)
        
        # prepare the command
        command = "zip -r --exclude="       \
                  + exclude                 \
                  + " " +self.__compressDir \
                  + "/" + name              \
                  + " " + inFile

        # execute the command to pack the binary
        changeDir = Cdir(workingDir)
        check_output([command], shell=True)
        del changeDir

#-------------------------------------------------------------------------------
    def getFiles(self):
        """
        Return the list of file/directory added 
        """
        return self.__listFile
        
