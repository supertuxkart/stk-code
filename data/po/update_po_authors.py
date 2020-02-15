#!/usr/bin/env python
# -*- coding: utf-8 -*-
# A simple script that adds all authors from transifex, which are
# listed in comments at the beginning of the file, to the 
# 'translator-credits' translations - where launchpad added them
# and which are shown in stk.
#
# First rename the transifex files:
# for i in supertuxkartpot_*; do mv $i ${i##supertuxkartpot_}; done
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
# 


import re
import sys

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: getpo_authors.py PATH_TO_PO_FILE")
        sys.exit(-1)

    for filename in sys.argv[1:]:
        print("Processing file %s" % filename)
        f = open(filename, "r")
        if not f:
            print("Can not find", filename)
            exit
        lines = f.readlines()

        f.close()

        # There are two sources of contributors:
        # 1) Entries in the header from transifex (after "Translators:"
        #    till the first msgid string.
        # 2) Entries as "translator-credits". These shouldn't be there,
        #    but sometimes occur (e.g. because an old version of this
        #    script was used that did not support these entries).
        # Assemble all those entries in one line
        
        found                  = 0
        contributions          = ""
        line_count             = 0
        new_authors            = []
        author_list            = []
        
        # Find new and old authors with a simple finite state machine:
        # ============================================================
        i = 0
        while i < len(lines):
            line = lines[i][:-1]   # remove \n
            i = i + 1
            if line=="# Translators:":
                while i <len(lines) and lines[i][:5]!="msgid":
                    line = lines[i][:-1]   # remove \n
                    if line [:14]!="# FIRST AUTHOR":
                        new_authors.append(line[2:])
                        author_list.append(line[2:])
                    i = i + 1
                
            elif line[:26] == "msgid \"translator-credits\"":
                line = lines[i][:-2]  # remove \"\n at end of line
                # Ignore the fist entry (which is "msgstr ...").
                authors = line.split("\\n")[1:]
                for j in range(len(authors)):
                    s = authors[j].strip()
                    if s!="":
                        author_list.append(s)
                # Then remove the msgid and msgstr
                del lines[i]      # msgstr
                del lines[i-1]    # msgid
                del lines[i-2]    # filename/line number info
                del lines[i-3]    # empty line

        
        # Delete all email addresses and URLs (esp. old launchpads)
        # ---------------------------------------------------------
        email = re.compile(r" *[^ ]+@[^ ]+ *")
        url   = re.compile(r" *https?://[^ ]*")
        for i, author in enumerate(author_list):
            g = email.search(author)
            if g:
                author_list[i] = author[:g.start()] +", " \
                               + author[g.end():]
            g = url.search(author)
            if g:
                author_list[i] = author[:g.start()] \
                               + author[g.end():]
                
        # Remove duplicated entries
        # -------------------------
        d = {}
        new_list = []
        for i, author in enumerate(author_list):
            if not d.get(author, None):
                new_list.append(author)
                d[author]=1
        author_list = new_list

        all_authors_string = reduce(lambda x,y: x+"\\n"+y, author_list, "")

        credits_line = "msgstr \"Launchpad Contributions:%s\"\n"%all_authors_string
            
        lines.append("\n")
        lines.append("#: src/states_screens/credits.cpp:209\n")
        lines.append("msgid \"translator-credits\"\n")
        lines.append(credits_line)


        # Overwrite old file
        f = open(filename, "w")
        for i in lines:
            f.write(i)
        f.close()
        
        print("Done with %s,"% filename)
            
