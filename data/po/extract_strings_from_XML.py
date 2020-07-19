import xml.dom.minidom
import sys
import codecs

f = open('./data/po/gui_strings.h', 'w')
f.write( codecs.BOM_UTF8 )

def traverse(file, node, isChallenge, isGP, isKart, isTrack, isAchievements, level=0):
  
    for e in node.childNodes:
        if e.localName == None:
            continue
    
        #print '    '*level, e.localName
        
        comment = None
        if e.hasAttribute("I18N"):
           comment = e.getAttribute("I18N")
        
        if e.localName == "subtitle" and e.hasAttribute("text") and len(e.getAttribute("text")) > 0:
               #print "Label=", e.getAttribute("name"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: Cutscene subtitle from " + file + "\n_(\"" + e.getAttribute("text") + "\")\n\n"
               else:
                   line += "//I18N: Cutscene subtitle from " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("text") + "\");\n\n"
               
               f.write( line.encode( "utf-8" ) )
        
        if isChallenge or isGP or isKart or isTrack or isAchievements:
           if isTrack and e.hasAttribute("internal") and e.getAttribute("internal") == "Y": continue
           if e.hasAttribute("name") and len(e.getAttribute("name")) > 0:
               #print "Label=", e.getAttribute("name"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: " + file + "\n_(\"" + e.getAttribute("name") + "\")\n\n"
               else:
                   line += "//I18N: File : " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("name") + "\");\n\n"
               
               f.write( line.encode( "utf-8" ) )
           
           # challenges and GPs can have a description file; karts don't
           if e.hasAttribute("description") and len(e.getAttribute("description")) > 0:
               # print "Label=", e.getAttribute("description"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: " + file + "\n_(\"" + e.getAttribute("description") + "\")\n\n"
               else:
                   line += "//I18N: File : " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("description") + "\");\n\n"
               
               f.write( line.encode( "utf-8" ) )
        else:
           if e.nodeName == "string" and e.hasAttribute("name") and e.getAttribute("name").startswith("po_") and e.firstChild is not None:
               line = "//I18N: In Android UI, " + e.getAttribute("name") + "\n_(\"" + e.firstChild.nodeValue  + "\")\n\n"
               f.write( line.encode( "utf-8" ) )
           elif e.hasAttribute("text") and len(e.getAttribute("text")) > 0:
               # print "Label=", e.getAttribute("text"), " Comment=", comment
               line = ""
               if comment == None:
                   line += "//I18N: " + file + "\n_(\"" + e.getAttribute("text").replace("\"", "\\\"") + "\")\n\n"
               else:
                   line += "//I18N: " + file + "\n//I18N: " + comment + "\n_(\"" + e.getAttribute("text") + "\");\n\n"
               f.write( line.encode( "utf-8" ) )

        # don't recurse in children nodes for karts, they can contain sounds, etc. that should not be translated
        if not isKart:
            traverse(file, e, isChallenge, isGP, isKart, isTrack, isAchievements, level+1)
      
filenames = sys.argv[1:]
for file in filenames:
    #print "Parsing", file
    
    isChallenge = False
    isGP = False
    isKart = False
    isTrack = False
    isAchievements = False
    
    if file.endswith(".challenge"):
        isChallenge = True
    if file.endswith(".grandprix"):
        isGP = True 
    if file.endswith("kart.xml"):
        isKart = True
    if file.endswith("track.xml"):
        isTrack = True
    if file.endswith("achievements.xml"):
        isAchievements = True
        
    try:
        doc = xml.dom.minidom.parse(file)
    except Exception as ex:
        print("============================================")
        print("/!\\ Expat doesn't like ", file, "! Error=", type(ex), " (", ex.args, ")")
        print("============================================")

    traverse(file, doc, isChallenge, isGP, isKart, isTrack, isAchievements)
    
