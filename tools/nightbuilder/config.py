#!/bin/python
# From Supertuxkart SVN revision $Revision$
# Copyright (C) 2012 Jean-manuel clemencon (samuncle)

# configuration for nightly builder

CONFIG = {
# Directory options
"WORKINGDIR"    :       "/home/tux/stkalpha/supertuxkart",
"BUILDDIR"      :       "/home/tux/stkalpha/supertuxkart/cmake_build",
"COMPRESSDIR"   :       "/home/tux/stkbeta",
"LOGDIR"        :       "/home/tux/stkalpha",
"SCRIPTDIR"     :       "/home/tux/stkalpha/nightbuilder",
# directory in the server 
"REMOTEDIR"     :       "/web/public/stknigtly",

# Build options
"JOBMAX"        :       3,

# FTP options
"FTPHOST"       :       "ftp.tux.net",
"FTPUSER"       :       "tux",
"FTPPASS"       :       "hell0",

# other
"PLATFORM"      :       "64",
"OS"            :       "lin",
"VERSION"       :       "svn"
}

# arguments by default
# PLEASE Don't edit this section if you don't know what you are doing. Thanks
ARG = {
"BIN"           :       True,
"DATA"          :       True,
"CLEAN"         :       False,
"SEND"          :       True,
"FORCE"         :       True,
"UPDATE"        :       True,
"JOB"           :       2,
"HELP"          :       False
}

