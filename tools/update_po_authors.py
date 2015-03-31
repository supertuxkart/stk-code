#!/bin/env python

# A simple script that adds all authors from transifex, which are
# listed in comments at the beginning of the file, to the 
# 'translator-credits' translations - where launchpad added them
# and which are shown in stk.
import re
import sys

if __name__ == "__main__":
    if len(sys.argv)!=2:
        print "Usage: getpo_authors.py PATH_TO_PO_FILE"
        sys.exit(-1)

    f = open(sys.argv[1], "r")
    if not f:
        print "Can not find", sys.argv[1]
        exit
    lines = f.readlines()

    f.close()

    authors = []
    found   = 0

    # Find all authors with a simple finite state machine:
    contributions = -1
    line_count    =  0
    for i in lines:
        line = i[:-1]   # remove \n
        if line=="# Translators:":
            found = 1
        elif found and line[:2]=="# ":
            authors.append(line[2:])
        elif line[:5]=="msgid":
            found = 0
        elif line[:31]== "msgstr \"Launchpad Contributions":
            contributions = line_count
        line_count = line_count + 1


    # Delete all email addresses - not sure if the authors
    # would want them to be published
    email=re.compile(" *<.*@.*\..*> *")   # one @ and one dot at least
    for i in range(len(authors)):
        g = email.search(authors[i])
        if g:
            authors[i] = authors[i][:g.start()]+authors[i][g.end():]

    # Now append the new authors to the Launchpad string
    if contributions==-1:
        print "Can not find old Launchpad Contributions string."
        sys.exit(-1)
    else:
        # Remove trailing "
        s=reduce(lambda x,y: x+"\\n  "+y, authors)
        lines[contributions] = lines[contributions][:-2]+"\\n  "+s+\
                               lines[contributions][-2:]

    f = open(sys.argv[1], "w")
    for i in lines:
        f.write(i)
    f.close()
            
