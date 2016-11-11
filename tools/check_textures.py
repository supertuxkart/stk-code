#!/usr/bin/env python

import os
import re
import struct

# Reads all texture names from the specified directory.
# Textures must end in jpg, png, or bmp.
# Returns a directory with the texture name as key.
def readAllTextures(dir, result):
    if type(dir)==type([]):
        for i in dir:
            readAllTextures(i,result)
        return
    
    re_is_texture = re.compile("^.*\.(jpg|png|bmp)$")
    for i in os.listdir(dir):
        if re_is_texture.match(i):
            result[i]=1

# -----------------------------------------------------------------------------
# Reads all texture names specified in a materials.xml file.

def readMaterialsXML(filename):
    textures = {}
    f = open(filename, "r")
    
    # Crude RE instead of XML parsing
    re_texture_name = re.compile("^ *<material name=\"([^\"]*)\"")
    for i in f.readlines():
        g = re_texture_name.match(i)
        if g:
            textures[g.groups(1)[0]] = 1

    return textures

# -----------------------------------------------------------------------------
def getTexturesFromB3D(filename, textures):
    f = open(filename, "r")
    s = f.read(4)
    if s!="BB3D":
        print filename,"is not a valid b3d file"
        f.close()
        return
    start_texs = 12
    f.seek(start_texs)
    s = f.read(4)
    if s!="TEXS":
        print "Can not handle '%s' b3d file - ignored."%filename
        f.close()
        return
    n = struct.unpack("<i", f.read(4))[0]   # Read end of section
    n = n - start_texs - 4  # number of bytes to read in tex section
    s = f.read(n)
    i = 0
    while i<n:
        tex_name=""
        while ord(s[i]):
            tex_name = tex_name+s[i]
            i=i+1
        textures[tex_name] = 1
        # Update the offst: add 1 byte string terminator,
        # and 7 int/float values
        i = i + 7*4 + 1
    f.close()
    return

# -----------------------------------------------------------------------------
# Reads all textures mentioned in a track.xml or kart.xml file (e.g. icons,
# screenshots, shadows)
def findTrackData(dir, textures, b3dfiles):
    f = open(dir+"track.xml", "r")
    if f:
        r_screenshot = re.compile("^ *screenshot *= \"(.*)\" *$")
    
        for i in f.readlines():
            g = r_screenshot.match(i)
            if g:
                textures[g.group(1)] = 1
        f.close()

    f = open(dir+"scene.xml", "r")
    if f:
        r_model      = re.compile(" model=\"((.*)\.b3d.?)\"")
        for i in f.readlines():
            g = r_model.search(i)
            if g:
                b3dfiles[g.groups(1)[0]] = 1
        f.close()
    
# -----------------------------------------------------------------------------
def findKartData(dir, textures, b3dfiles):
    f = open(dir+"scene.xml", "r")
    if f:
        r_model = re.compile(" model=\"((.*)\.b3d.?)\"")
        for i in f.readlines():
            g = r_model.search(i)
            if g:
                b3dfiles[g.groups(1)[0]] = 1
        f.close()
        return
    else:
        f = open(dir+"scene.xml", "r")
        if not f: return
        print "WARNING"
    if 1:
        print "WARNING - kart.xml not done yet"
        f = open(dir+"kart.xml", "r")
        if not f: return
        r_screenshot = re.compile("^ *screenshot *= \"(.*)\" *$")
        for i in f.readlines():
            g = r_screenshot.match(i)
            if g:
                textures[g.group(1)] = 1
        f.close()
        return
    
                    
# -----------------------------------------------------------------------------
# Checks if all textures for a specified track- or kart-directory
# are actually used.

def checkDir(dir, shared_textures):

    # First read all *png/jpg/bmp files
    # ---------------------------------
    existing_textures = {}
    readAllTextures(dir, existing_textures)

    # Find all b3d files in the directory (print warnings for b3dz)
    # -------------------------------------------------------------
    b3d_files_in_dir = {}
    for i in os.listdir(dir):
        if i[-4:]==".b3d":
            b3d_files_in_dir[i] = 1
        elif i[-5:]==".b3dz":
            print "Can't handle file '%s'."%i
            
    # Find all textures listed in materials.xml
    # -----------------------------------------
    materials = readMaterialsXML(dir+"/materials.xml")

    # Find all textures in track.xml and kart.xml files
    # -------------------------------------------------
    used_textures = {}
    used_b3d_files = {}
    findTrackData(dir, used_textures, used_b3d_files)
    #findKartData(dir, used_textures, used_b3d_files)

    # 1) Check if there are any missing b3d files
    # ===========================================
    for i in used_b3d_files.keys():
        if not b3d_files_in_dir.get(i):
            print "File '%s' is missing."%(dir+i)
        
    # 2) Check if there are any unnecessary b3d files
    # ===============================================
    for i in b3d_files_in_dir:
        if not used_b3d_files.get(i):
            print "File '%s' is not used."%i
            continue
        del used_b3d_files[i]
        # Load all textures used in this b3d file
        getTexturesFromB3D(dir+i, used_textures)
        
    # 3) Check if all textures used can be found
    # ==========================================
    for i in used_textures:
        if not existing_textures.get(i)==1:
            if not shared_textures.get(i):
                print "Cannot find texture '%s'."%i
                continue
        else:
            del existing_textures[i]
            

    for i in existing_textures:
        print "Texture '%s' is not used anywhere."%(dir+i)
            
    # Now check that all entries in materials.xml are indeed used and exist



# =============================================================================

if __name__=="__main__":
    assets = "../stk-assets/"
    shared_textures = {}
    readAllTextures([assets+"textures",
                     assets+"/textures/deprecated"],
                    shared_textures)
    
    for i in os.listdir(assets+"tracks"):
        checkDir(assets+"tracks/"+i+"/", shared_textures)
        break

    #for i in os.listdir(assets+"karts"):
    #    checkDir(assets+"karts/"+i+"/", shared_textures)
