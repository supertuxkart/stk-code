#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)
# Class used to package the project
################################################################################
from subprocess import check_output
import os
from utils import *

class Send:
    """
    Interface for the network
    """
    
    def __init__ (self, ftpHost, ftpUser, ftpPass, ftpFileDir):
        """
        Constructor of the builder class
        """

        # Get the configuration for the FTP connection
        lines = [
        'FTP_HOST="' + ftpHost + '"\n',
        'FTP_USER="' + ftpUser + '"\n',
        'FTP_PASS="' + ftpPass + '"\n',
        '\n',
        'ftp -n -p $FTP_HOST << EOF\n',
        'user $FTP_USER $FTP_PASS\n'
        ]

        # open the ftp script
        self.__ftpCmd = open("ftp.sh", "w")
        self.__ftpCmd.writelines(lines)
        self.__ftpFileDir = ftpFileDir
    
    def add(self, filename, localDir, remoteDir):
        """
        Add a file to the sender
        """
        command = "put "                \
                + localDir              \
                + "/" + filename        \
                + " " + remoteDir       \
                + "/" + filename        \
                + "\n"
        self.__ftpCmd.write(command)
        
    
    def send(self):
        """
        Send files previously added 
        """
        self.__ftpCmd.write("bye\nEOF\n")
        self.__ftpCmd.close()
        check_output([self.__ftpFileDir+"/ftp.sh"], shell=True)
        #os.system("./ftp.sh")
        
    def alert(self):
        """
        Send an e-mail alert to the mailing list
        """
        #TODO




