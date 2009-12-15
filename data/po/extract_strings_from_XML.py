import xml.dom.minidom
import sys

f = open('./data/po/gui_strings.h', 'w')

def traverse(node, level=0):
  
    for e in node.childNodes:
        if e.localName == None:
            continue
    
        #print '    '*level, e.localName
        
        comment = None
        if e.hasAttribute("I18N"):
           comment = e.getAttribute("I18N")
            
        if e.hasAttribute("text"):
            # print "Label=", e.getAttribute("text"), " Comment=", comment
            line = ""
            if comment == None:
                line += "_(\"" + e.getAttribute("text") + "\")\n\n"
            else:
                line += "//I18N: " + comment + "\n_(\"" + e.getAttribute("text") + "\");\n\n"
            
            f.write( line )

        
        traverse(e, level+1)
      
filenames = sys.argv[1:]
for file in filenames:
    print "Parsing", file
    doc = xml.dom.minidom.parse(file)
    traverse(doc)
    
