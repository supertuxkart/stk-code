import xml.dom.minidom
import sys

f = open('./data/po/gui_strings.h', 'w')

def traverse(file, node, isChallenge, isGP, level=0):
  
    for e in node.childNodes:
        if e.localName == None:
            continue
    
        #print '    '*level, e.localName
        
        comment = None
        if e.hasAttribute("I18N"):
           comment = e.getAttribute("I18N")
         
        if isChallenge or isGP:
           if e.hasAttribute("name"):
               # print "Label=", e.getAttribute("name"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: " + file + "\n_(\"" + e.getAttribute("name") + "\")\n\n"
               else:
                   line += "//I18N: File : " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("name") + "\");\n\n"
               
               f.write( line )
               
           if e.hasAttribute("description"):
               # print "Label=", e.getAttribute("description"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: " + file + "\n_(\"" + e.getAttribute("description") + "\")\n\n"
               else:
                   line += "//I18N: File : " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("description") + "\");\n\n"
               
               f.write( line )
        else:
           if e.hasAttribute("text"):
               # print "Label=", e.getAttribute("text"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: " + file + "\n_(\"" + e.getAttribute("text") + "\")\n\n"
               else:
                   line += "//I18N: " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("text") + "\");\n\n"
               
               f.write( line )

        
        traverse(file, e, isChallenge, isGP, level+1)
      
filenames = sys.argv[1:]
for file in filenames:
    #print "Parsing", file
    
    isChallenge = False
    isGP = False
    
    if file.endswith(".challenge"):
        isChallenge = True
    if file.endswith(".grandprix"):
        isGP = True 
        
    try:
        doc = xml.dom.minidom.parse(file)
    except:
        print "============================================"
        print "/!\\ Expat doesn't like ", file, "!"
        print "============================================"

    traverse(file, doc, isChallenge, isGP)
    
