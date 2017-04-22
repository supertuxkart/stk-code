#!/usr/bin/env python
# -*- coding: utf-8 -*-
# A simple script that adds all authors from transifex, which are
# listed in comments at the beginning of the file, to the 
# 'translator-credits' translations - where launchpad added them
# and which are shown in stk.
#
# Usage:  update_po_authors.py  PATH_TO/LANGUAGE.po
#
# IMPORTANT note: this script must be run on a file downloaded
#           from transifex, not on a file on which this script had
#           been run before (otherwise the transifex authors would
#           be added again and again)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# Less important note: The specified file is overwritten without
#           a backup! Make sure the git status is unmodified.

import re
import sys

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage: getpo_authors.py PATH_TO_PO_FILE"
        sys.exit(-1)

    for filename in sys.argv[1:]:
        print("Processing file ", filename)
        f = open(filename, "r")
        if not f:
            print "Can not find", filename
            exit
        lines = f.readlines()

        f.close()

        new_authors = []
        found        = 0

        # Find all authors with a simple finite state machine:
        contributions = -1
        line_count    =  0
        for i in lines:
            line = i[:-1]   # remove \n
            if line=="# Translators:":
                found = 1
            elif found and line[:2]=="# " and line [:14]!="# FIRST AUTHOR":
                new_authors.append(line[2:])
            elif line[:5]=="msgid":
                found = 0
            elif line[:31]== "msgstr \"Launchpad Contributions":
                contributions = line_count
            line_count = line_count + 1


        # Delete all email addresses - not sure if the authors
        # would want them to be published
        email=re.compile(" *<.*@.*\..*> *")   # one @ and one dot at least
        for i in range(len(new_authors)):
            g = email.search(new_authors[i])
            if g:
                new_authors[i] = new_authors[i][:g.start()] \
                               + new_authors[i][g.end():]

        # Get the old authors from the translator-credits string:
        if contributions>0:
            # Ignore the first entry, which is "msgstr ...", and the
            # last two characters, which are the '"\n'.
            old_authors = lines[contributions][:-2].split("\\n")[1:]
            for i in range(len(old_authors)):
                old_authors[i] = old_authors[i].strip()
        else:
            old_authors=[]
        
        all_authors = old_authors + new_authors;
        all_authors = sorted(all_authors, key=lambda x: x.lower())
        all_authors_string = reduce(lambda x,y: x+"\\n"+y, all_authors, "")

        credits_line = "msgstr \"Launchpad Contributions:%s\"\n"%all_authors_string
        # If no old authors exists, write a new entry:
        if contributions==-1:
            lines.append("\n")
            lines.append("#: src/states_screens/credits.cpp:209\n")
            lines.append("msgid \"translator-credits\"\n")
            lines.append(credits_line)
        else:
            # Otherwise just replace the old contribution string
            lines[contributions] = credits_line


        # Overwrite old file
        f = open(filename, "w")
        for i in lines:
            f.write(i)
        f.close()
        
        print("Done with ", filename)
            
