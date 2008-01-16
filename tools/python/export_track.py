#!BPY

""" Registration info for Blender menus:
Name: 'Super Tux Kart Track'
Blender: 233
Group: 'Export'
Submenu: 'All meshes and empties ...' all
Submenu: 'Only selected ...' sel
Submenu: 'Configure' config
Tip: 'Create Super Tux Kart racing tracks.'
"""

# --------------------------------------------------------------------------
# This script turns the AC3D exporter script into a TuxKart track exporter.
# --------------------------------------------------------------------------
# ***** BEGIN GPL LICENSE BLOCK *****
#
# Copyright (C) 2004: Willian P. Germano, wgermano _at_ ig.com.br
# Copyright (C) 2006: Eduardo Hernández, coz.eduardo.hernandez _at_ gmail.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# ***** END GPL LICENCE BLOCK *****
# --------------------------------------------------------------------------

VERSION = '0.2'

HELPTEXT = HELPTEXT_INTRO = [
'This script can export racing tracks with all needed data for TuxKart.',
'',
'TuxKart is an open source 3D kart racing game by Steve and Oliver Baker.',
'It is written in portable C++ using OpenGL and Steve\'s PLib 3D gaming library.',
'',
'Links:',
'TuxKart: tuxkart.sf.net   PLib: plib.sf.net   OpenGL: opengl.org',
'',
'The game\'s site contains a text describing how to add new tracks.',
'Please read it to understand better how to use this script.',
]

HELPTEXT_DRV = [
'To export the drive path file you need to create two mesh objects,',
'which are the left and right borders of the track. They must be only',
'linked 2D line segments. The left border mesh object must',
'be called "DRV_LEFT" and the right border mesh object is to be',
'called "DRV_RIGHT".',
'',
'Hints to create the drive line in the 3D View window (use TOP VIEW):',
'',
'- select the track mesh (the actual path);',
'- enter edit mode, and select all the vertex of one of the borders;',
'- use the P-KEY to separate that border from the main mesh;',
'- select the other border and separate it;',
'- change the name of the mesh objects to DRV_LEFT and DRV_RIGHT;',
'',
'To write the vertices in the correct order, the script starts with the',
'closest one to (0,0,0), looks for the edge that goes the most "up" ('
'increasing y coordinate) then follows the line segments till the last one.'
]

HELPTEXT_LOC = [
'To put items on the track add Empties (SPACEBAR->ADD->EMPTY)',
'where you want them to appear and give them a proper name.',
'Examples:',
'(x,y,z are coordinates and h,p,r are rotation angles)'
'',
'- sign.ac is exported as .loc line: "sign.ac",x,y,z,h,p,r',
'- sign.ac{Z}: "sign.ac",x,y,{},h,p,r',
'- sign.ac{ZPR}: "sign.ac",x,y,{},h,{},{}',
'- RHERRING exported as: RHERRING,x,y,z',
'',
'So (did you read the text about this at TuxKart\'s site?):',
'- to have z,p or r calculated by the game, append them inside {};',
'- names without an extension (.something) are considered hardcoded',
'  game objects, like GHERRING (default if name is left as Empty);',
'- item examples: zipper.ac, RHERRING, GHERRING, YHERRING;',
'- don\'t worry about the .001 etc. extensions appended by Blender.',
'',
'You can assign a song file for your track in the Config screen.'
]

HELPTEXT_AC = [
'The script uses the AC3D exporter.',
'',
'AC3D is a shareware 3D modeler with a simple text format (.ac).',
'',
'There are a few options you can configure:',
'- skip data: set it if you don\'t want mesh names (ME:, not OB: field)',
'  to be exported as strings for AC\'s "data" tags (19 chars max);',
'- rgb mirror color can be exported as ambient and/or emissive if needed,',
'  since Blender handles these differently;',
'- default mat: a default (white) material is added if some mesh was',
'  left without mats -- it\'s better to always add your own materials...',
'',
'To export mesh groups, parent each set to an Empty that has the suffix "GR_".'
]

import Blender
from Blender import sys as bsys
from Blender import Mathutils
from math import sqrt

# Globals / config options:
ARG = __script__['arg'] # user selected argument
HELPME = CONFIG = 0 # secondary screens
TK_ALL = 0
TK_DRV = 1
TK_LOC = 0
TK_TRK = 0
TK_PATH = bsys.dirname(Blender.Get('filename')) + bsys.sep
TK_TUNE = ''
AC_SKIP_DATA = 0
AC_MIRCOL_AS_AMB = 0
AC_MIRCOL_AS_EMIS = 0
AC_ADD_DEFAULT_MAT = 1

# Looking for a saved key in Blender.Registry dict:
rd = Blender.Registry.GetKey('TuxKart')
if rd:
  TK_ALL = rd['TK_ALL']
  TK_DRV = rd['TK_DRV']
  TK_LOC = rd['TK_LOC']
  TK_TRK = rd['TK_TRK']
  TK_PATH = rd['TK_PATH']
  TK_TUNE = rd['TK_TUNE']
  AC_SKIP_DATA = rd['AC_SKIP_DATA']
  AC_MIRCOL_AS_AMB = rd['AC_MIRCOL_AS_AMB']
  AC_MIRCOL_AS_EMIS = rd['AC_MIRCOL_AS_EMIS']
  AC_ADD_DEFAULT_MAT = rd['AC_ADD_DEFAULT_MAT']

if "DRV_LEFT" and "DRV_RIGHT"  in Blender.Object.Get():
  TK_DRV = 1 # found both DRV meshes

def update_RegistryInfo():
  d = {}
  d['TK_ALL'] = TK_ALL
  d['TK_DRV'] = TK_DRV
  d['TK_LOC'] = TK_LOC
  d['TK_TRK'] = TK_TRK
  d['TK_PATH'] = TK_PATH
  d['TK_TUNE'] = TK_TUNE
  d['AC_SKIP_DATA'] = AC_SKIP_DATA
  d['AC_MIRCOL_AS_AMB'] = AC_MIRCOL_AS_AMB
  d['AC_MIRCOL_AS_EMIS'] = AC_MIRCOL_AS_EMIS
  d['AC_ADD_DEFAULT_MAT'] = AC_ADD_DEFAULT_MAT
  Blender.Registry.SetKey('TuxKart', d)

# The default material to be used when necessary (see AC_ADD_DEFAULT_MAT)
DEFAULT_MAT = \
'MATERIAL "DefaultWhite" rgb 1 1 1  amb 1 1 1  emis 0 0 0 \
 spec 0.5 0.5 0.5 shi 64  trans 0'

# This transformation aligns Blender and AC3D coordinate systems:
acmatrix = Mathutils.Matrix([1,0,0,0], [0,0,-1,0], [0,1,0,0], [0,0,0,1])

def Round(f):
  r = round(f,6) # precision set to 10e-06
  if r == int(r):
    return str(int(r))
  else:
    return str(r)
 
def transform_verts(verts, m):
  vecs = []
  for v in verts:
    vec = Mathutils.Vector([v[0],v[1],v[2], 1])
    vecs.append(vec * m)
  return vecs

# ---

def WriteLocation(objs, filename):
  "Write the .loc file"
  filename = Blender.sys.splitext(filename)[0]
  trackname = Blender.sys.basename(filename)+".ac"
  filename += '.loc'
  locfile = file(filename, "w")

  print "Writing %s file..." % filename

  locfile.write("# Created by tuxkart.py v%s Blender Python script.\n#\n" \
    % VERSION)
  locfile.write('"%s",0,0,0,0,0,0\n' % trackname)

  rad2deg = 180/3.1415926535 #from radians to degrees
  for item in objs:
    x,y,z = map(str, map(Round, item.loc))
    rx,ry,rz = map(lambda x: rad2deg*x, item.rot)
    h,p,r = map(str, map(Round, [rz,rx,ry]))

    name = item.name.split(".")
    hardcoded = 1 # name has extension (like .ac) ?
    if len(name) == 1:
      name = name[0]
    else:
      if name[-1].isdigit():
        name = name[:-1]
        if name == 'Empty': name = 'GHERRING'
      if len(name) == 1:
        name = name[0]
      else:
        name = ".".join(name)
        hardcoded = 0 # has extension, isn't harcoded
    if name.find("{") > 0:
      name, encoded = name.split("{", 1)
      encoded = encoded.split("}")[0].lower()
      if encoded.find("z") >= 0: z = '{}'
      if encoded.find("p") >= 0: p = '{}'
      if encoded.find("r") >= 0: r = '{}'

    locline = '%s,%s,%s' % (x,y,z)
    if not hardcoded:
      name = '"'+name+'"'
      locline += ',%s,%s,%s' % (h,p,r)
    locfile.write('%s,%s\n' % (name, locline))

  if TK_TUNE:
    locfile.write('MUSIC "%s"\n' % TK_TUNE)
  locfile.close()


def WriteLeftDriveLine(obj, filename):
  "Write the .drvl file"
  mesh = obj.getData()
  mesh.transform(obj.getMatrix())
  for f in mesh.faces:
    if len(f.v) != 2:
      print "Can't export Drive Line .drvl file: invalid mesh."
      print "The mesh must be made of linked line segments only."
      return

  filename = Blender.sys.splitext(filename)[0]
  filename += '.drvl'
  drvfile = file(filename, "w")
  print "Writing drive line file: %s" % filename
  verts = mesh.verts

  shortest_distance = 100000
  for v in verts:
   #find vertex closest to 0,0
    distance = sqrt(v[0]**2 + v[1]**2)
    if distance <  shortest_distance:
      shortest_distance = distance
      first_vertex = v

  #Find the two vertex that are linked to the first vertex
  e_v = []
  edges = mesh.edges

  for e in edges:
    if first_vertex == e.v1:
      e_v.append(e.v2)
    elif first_vertex == e.v2:
      e_v.append(e.v1)
 
  second_vertex = first_vertex
  #Find the vertex which has the higher Y value
  for v in e_v:
    if v[1] > second_vertex[1]:
      second_vertex = v
 
  if second_vertex == first_vertex:
    print "Can't export Drive Line .drvl file: invalid mesh."
    print "The mesh only has one point."
    return

  sorted_vertex = [first_vertex, second_vertex]

  prev_vertex = first_vertex
  curr_vertex = second_vertex

  count = 2
  #Find the edge that contains the second vertex but does not has first vertex
  while count != len(verts):
    for e in edges:
      if prev_vertex != e.v1 and prev_vertex != e.v2:
        #Find which vertex is the next_vertex
        if curr_vertex == e.v1:
          prev_vertex = curr_vertex
          curr_vertex = e.v2
          sorted_vertex.append(curr_vertex)
          count = count + 1
          break
        elif curr_vertex == e.v2:
          prev_vertex = curr_vertex
          curr_vertex = e.v1
          sorted_vertex.append(curr_vertex)
          count = count + 1
          break

  for v in sorted_vertex:
   drvfile.write('%f,%f,%f\n' % (v[0], v[1], v[2]))
 
  drvfile.close() 

def WriteRightDriveLine(obj, filename):
  "Write the .drvl file"
  mesh = obj.getData()
  mesh.transform(obj.getMatrix())
  for f in mesh.faces:
    if len(f.v) != 2:
      print "Can't export Drive Line .drvr file: invalid mesh."
      print "The mesh must be made of linked line segments only."
      return

  filename = Blender.sys.splitext(filename)[0]
  filename += '.drvr'
  drvfile = file(filename, "w")
  print "Writing drive line file: %s" % filename
  verts = mesh.verts

  shortest_distance = 100000
  for v in verts:
   #find vertex closest to 0,0
    distance = sqrt(v[0]**2 + v[1]**2)
    if distance <  shortest_distance:
      shortest_distance = distance
      first_vertex = v

  #Find the two vertex that are linked to the first vertex
  e_v = []
  edges = mesh.edges

  for e in edges:
    if first_vertex == e.v1:
      e_v.append(e.v2)
    elif first_vertex == e.v2:
      e_v.append(e.v1)
 
  second_vertex = first_vertex
  #Find the vertex which has the higher Y value
  for v in e_v:
    if v[1] > second_vertex[1]:
      second_vertex = v
 
  if second_vertex == first_vertex:
    print "Can't export Drive Line .drvr file: invalid mesh."
    print "The mesh only has one point."
    return

  sorted_vertex = [first_vertex, second_vertex]

  prev_vertex = first_vertex
  curr_vertex = second_vertex

  count = 2
  #Find the edge that contains the second vertex but does not has first vertex
  while count != len(verts):
    for e in edges:
      if prev_vertex != e.v1 and prev_vertex != e.v2:
        #Find which vertex is the next_vertex
        if curr_vertex == e.v1:
          prev_vertex = curr_vertex
          curr_vertex = e.v2
          sorted_vertex.append(curr_vertex)
          count = count + 1
          break
        elif curr_vertex == e.v2:
          prev_vertex = curr_vertex
          curr_vertex = e.v1
          sorted_vertex.append(curr_vertex)
          count = count + 1
          break

  for v in sorted_vertex:
   drvfile.write('%f,%f,%f\n' % (v[0], v[1], v[2]))
 
  drvfile.close() 


class Track: # the ac3d exporter part

  def __init__(self, bl_objlist, filename):

    from Blender.sys import time
    TIME = time()
    global ARG, AC_SKIP_DATA, AC_ADD_DEFAULT_MAT, DEFAULT_MAT

    print 'Exporting track model(s) to AC3D format...'

    header = 'AC3Db'
    self.buf = ''
    self.mbuf = ''
    world_kids = 0
    self.mlist = []
    kids_dict = {}
    objlist = []
    bl_objlist2 = []

    if filename.find('.ac', -3) <= 0: filename += '.ac'

    try:
      file = open(filename, 'w')
    except IOError, (errno, strerror):
      errmsg = "IOError #%s" % errno
      errmsg = errmsg + "%t|" + strerror
      Blender.Draw.PupMenu(errmsg)
      return None

    file.write(header+'\n')

    for obj in bl_objlist:
      if obj.getType() != 'Mesh' and obj.getType() != 'Empty':
        continue
      else: kids_dict[obj.name] = 0
      if obj.getParent() == None:
        objlist.append(obj)
      else:
        bl_objlist2.append(obj)

    bl_objlist = bl_objlist2[:]
    world_kids = len(objlist)

    while bl_objlist2:
      for obj in bl_objlist:
        obj2 = obj
        dad = obj.getParent()
        kids_dict[dad.name] += 1
        while dad.name not in objlist:
          obj2 = dad
          dad = dad.getParent()
          kids_dict[dad.name] += 1
        objlist.insert(objlist.index(dad.name)+1, obj2.name)
        bl_objlist2.remove(obj2)

    meshlist = [o for o in objlist if o.getType() == 'Mesh']

    self.MATERIALS(meshlist)
    if not self.mbuf or AC_ADD_DEFAULT_MAT:
      self.mbuf = DEFAULT_MAT + '\n' + self.mbuf
    file.write(self.mbuf)

    file.write('OBJECT world\nkids %s\n' % world_kids)

    for obj in objlist:
      self.obj = obj

      if obj.getType == 'Empty':
        file.write('OBJECT group\n')
        emptyname = obj.name # skip the GR_ prefix
        if len(emptyname) > 3: emptyname = emptyname[3:]
        file.write(emptyname+'\n')
        #self.rot(obj.rot)
        #self.loc(obj.loc)
      else:
        mesh = self.mesh = obj.getData()
        file.write('OBJECT poly\nname "%s"\n' % obj.name)
        if not AC_SKIP_DATA:
          file.write('data %s\n%s\n' % (len(mesh.name), mesh.name))
        texline = self.texture(mesh.faces)
        if texline: file.write(texline)
        self.numvert(mesh.verts, obj.getMatrix(), file)
        self.numsurf(mesh.faces, mesh.hasFaceUV(), file)

      file.write('kids %s\n' % kids_dict[obj.name])

    file.close()
    print "Done. Saved to %s\n" % filename

  def MATERIALS(self, meshlist):
    for meobj in meshlist:
      me = meobj.getData()
      mat = me.materials
      mbuf = []
      mlist = self.mlist
      for m in xrange(len(mat)):
        name = mat[m].name
        try:
          mlist.index(name)
        except ValueError:
          mlist.append(name)
          M = Blender.Material.Get(name)
          material = 'MATERIAL "%s"' % name
          mirCol = "%s %s %s" % (Round(M.mirCol[0]), Round(M.mirCol[1]),
            Round(M.mirCol[2]))
          rgb = "rgb %s %s %s" % (Round(M.R), Round(M.G), Round(M.B))
          amb = "amb %s %s %s" % (Round(M.amb), Round(M.amb), Round(M.amb))
          if AC_MIRCOL_AS_AMB:
            amb = "amb %s" % mirCol 
          emis = "emis 0 0 0"
          if AC_MIRCOL_AS_EMIS:
            emis = "emis %s" % mirCol
          spec = "spec %s %s %s" % (Round(M.specCol[0]),
             Round(M.specCol[1]), Round(M.specCol[2]))
          shi = "shi 72"
          trans = "trans %s" % (Round(1 - M.alpha))
          mbuf.append("%s %s %s %s %s %s %s\n" \
            % (material, rgb, amb, emis, spec, shi, trans))
      self.mlist = mlist
      self.mbuf += "".join(mbuf)

  def texture(self, faces):
    tex = []
    for f in faces:
      if f.image and f.image.name not in tex:
        tex.append(f.image.name)
    if tex:
      if len(tex) > 1:
        print "\nAC3Db format supports only one texture per object."
        print "Object %s -- using only the first one: %s\n" % \
          (self.obj.name, tex[0])
      image = Blender.Image.Get(tex[0])
      buf = 'texture "%s"\n' % image.filename
      xrep = image.xrep
      yrep = image.yrep
      buf += 'texrep %s %s\n' % (xrep, yrep)
      return buf

  def rot(self, matrix):
    rot = ''
    not_I = 0
    for i in [0, 1, 2]:
      r = map(Round, matrix[i])
      not_I += (r[0] != '0.0')+(r[1] != '0.0')+(r[2] != '0.0')
      not_I -= (r[i] == '1.0')
      for j in [0, 1, 2]:
        rot = "%s %s" % (rot, r[j])
    if not_I:
      rot = rot.strip()
      buf = 'rot %s\n' % rot
      self.buf = self.buf + buf
        
  def loc(self, loc):
    loc = map(Round, loc)
    if loc[0] or loc[1] or loc[2]:
      buf = 'loc %s %s %s\n' % (loc[0], loc[1], loc[2])
      self.buf = self.buf + buf

  def numvert(self, verts, matrix, file):
    file.write("numvert %s\n" % len(verts))
    m = matrix * acmatrix
    verts = transform_verts(verts, m)
    for v in verts:
      v0, v1, v2 = Round(v[0]), Round(v[1]), Round(v[2])
      file.write("%s %s %s\n" % (v0, v1, v2))

  def numsurf(self, faces, hasFaceUV, file):

    global AC_ADD_DEFAULT_MAT
 
    file.write("numsurf %s\n" % len(faces))

    mlist = self.mlist
    indexerror = 0
    omlist = {}
    objmats = self.mesh.materials
    for i in range(len(objmats)):
      objmats[i] = objmats[i].name
    for f in faces:
      m_idx = f.materialIndex
      try:
        m_idx = mlist.index(objmats[m_idx])
      except IndexError:
        if not indexerror:
          print "\nNotice: object " + self.obj.name + \
            " has at least one material *index* assigned\n" \
            "\tbut not defined (not linked to an existing material).\n" \
            "\tThis can make some faces be exported with a wrong color.\n" \
            "\tYou can fix the problem in the Edit Buttons Window (F9).\n"
          indexerror = 1
        m_idx = 0
      refs = len(f)
      flaglow = (refs == 2) << 1
      two_side = f.mode & Blender.NMesh.FaceModes['TWOSIDE']
      two_side = (two_side > 0) << 1
      flaghigh = f.smooth | two_side
      surfstr = "SURF 0x%d%d\n" % (flaghigh, flaglow)
      if AC_ADD_DEFAULT_MAT and objmats: m_idx += 1
      matstr = "mat %s\n" % m_idx
      refstr = "refs %s\n" % refs
      u, v, vi = 0, 0, 0
      fvstr = ''
      for vert in f.v:
        fvstr = fvstr + str(self.mesh.verts.index(vert))
        if hasFaceUV:
          u = f.uv[vi][0]
          v = f.uv[vi][1]
          vi += 1
        fvstr = fvstr + " %s %s\n" % (u, v)
      file.write(surfstr + matstr + refstr + fvstr)

# End of Class Track

from Blender import Draw, BGL
from Blender.Window import FileSelector

# Special Buttons
BUTPATH = Draw.Create(TK_PATH)
BUTTUNE = Draw.Create(TK_TUNE)
BUTTRK = Draw.Create(TK_TRK)
BUTDRV = Draw.Create(TK_DRV)
BUTALL = Draw.Create(TK_ALL)
BUTLOC = Draw.Create(TK_LOC)
HTRK = Draw.Create(0)
HDRV = Draw.Create(0)
HLOC = Draw.Create(0)
HINTRO = Draw.Create(0)

HTOG = HTOG_INTRO = 0
HTOG_TRK = 1
HTOG_LOC = 2
HTOG_DRV = 3

# Button Events
EVT_CONFIG = 1
EVT_AC_DMAT = 2
EVT_AC_SKIP = 3
EVT_AC_MIR = 4
EVT_AC_EMIS = 5
EVT_AC_ALL = 6
EVT_AC_SEL = 7
EVT_HELP = 10
EVT_HELP_DRV = 11
EVT_HELP_LOC = 12
EVT_HELP_TRK = 13
EVT_HELP_INTRO = 14
EVT_ALL = 20
EVT_TRK = 21
EVT_DRV = 22
EVT_LOC = 23
EVT_PATH = 24
EVT_BROWSE = 25
EVT_TUNE = 26
EVT_TUNEBROWSE = 27
EVT_EXPORT = 30
EVT_EXIT = 31

def gui(): # gui drawing callback
  global AC_SKIP_DATA, AC_MIRCOL_AS_AMB, AC_MIRCOL_AS_EMIS, AC_ADD_DEFAULT_MAT
  global HELPME, CONFIG, HELPTEXT, HTRK, HDRV, HLOC, HINTRO
  global HTOG, HTOG_INTRO, HTOG_TRK, HTOG_LOC, HTOG_DRV
  global BUTPATH, BUTTRK, BUTDRV, BUTALL, BUTLOC, BUTTUNE

  if HELPME: # help screen
    BGL.glClearColor(0.4,0.5,0.8,1)
    BGL.glClear(BGL.GL_COLOR_BUFFER_BIT)
    BGL.glColor3f(1,1,1)
    
    x = 55
    y = 50+20*len(HELPTEXT)
    for line in HELPTEXT:
      y -= 20
      BGL.glRasterPos2i(x, y)
      Draw.Text(line)
    w = 35
    h = 20
    y = 10
    HINTRO = Draw.Toggle("intro", EVT_HELP_INTRO, x, y, w, h,(HTOG==HTOG_INTRO),
      "Intro Help screen.")
    HLOC = Draw.Toggle("loc", EVT_HELP_LOC, x+w, y, w, h, (HTOG==HTOG_LOC),
      "Help for location (.loc) file.")
    HTRK = Draw.Toggle("track", EVT_HELP_TRK, x+2*w, y, w, h, (HTOG==HTOG_TRK),
      "Help for the mesh exporter.")
    HDRV = Draw.Toggle("drv", EVT_HELP_DRV, x+3*w, y, w, h, (HTOG==HTOG_DRV),
      "Help for drive line (.drv) file.")
    Draw.Button("back", EVT_HELP, x+4*w, y, w, h, "Return to main screen.")

  elif CONFIG: # config screen
    BGL.glClearColor(0.4,0.5,0.8,1)
    BGL.glClear(BGL.GL_COLOR_BUFFER_BIT)
    BGL.glColor3f(0.9,0.85,0.75)
    BUTTUNE = Draw.String('song: ', EVT_TUNE, 10, 10, 210, 20, \
      BUTTUNE.val, 150, 'Song filename for your race track.')
    Draw.PushButton('browse...', EVT_TUNEBROWSE, 222, 10, 53, 20,
      'Open file selector (select any file in the chosen dir)') 
    x = 15
    y = 40
    w = 80
    h = 20
    BGL.glRecti(x-4,y-5,x+205,y+84)
    BGL.glColor3f(1,1,1)
    Draw.Button("BACK", EVT_CONFIG, x+w+127, y-5, 53, 30,
      "Click to return to main screen.")
    Draw.Button("Export Selected...", EVT_AC_SEL, x+w+10, y+4, 110, 30,
      "Export now selected meshes to an AC3D file.")
    Draw.Button("Export All...", EVT_AC_ALL, x+w+10, y+44, 110, 30,
      "Export now all meshes to an AC3D file.")
    Draw.Toggle("Mir2Emis", EVT_AC_EMIS, x, y, w, h, AC_MIRCOL_AS_EMIS,
      "Get AC3D's emissive RGB color for each object from its mirror color "
      "in Blender.")
    Draw.Toggle("Mir2Amb", EVT_AC_MIR, x, y+h, w, h, AC_MIRCOL_AS_AMB,
      "Get AC3D's ambient RGB color for each object from its mirror color "
      "in Blender.")
    Draw.Toggle("Skip data", EVT_AC_SKIP, x, y+2*h, w, h, AC_SKIP_DATA,
      "Don't export mesh names as 'data' info.")
    Draw.Toggle("Default mat", EVT_AC_DMAT, x, y+3*h, w, h, AC_ADD_DEFAULT_MAT,
      "Objects without materials assigned get a default (white) one "
      "automatically.")
    BGL.glRasterPos2i(x-4, y+5*h)
    Draw.Text("Additional Configuration")

  else: # main screen

    # let's count our objects first:
    meshnr = meshsel = emptynr = emptysel = 0
    ob = Blender.Object.Get()
    for o in ob:
      if o.getType() == "Mesh":
        meshnr += 1
        if o.sel: meshsel += 1
      elif o.getType() == "Empty":
        emptynr += 1
        if o.sel: emptysel += 1
    obinfo = "%d/%d meshes" % (meshsel, meshnr)
    if emptynr:
      obinfo += ", %d/%d empties" % (emptysel, emptynr)
    if len(obinfo) < 27: obinfo = 'Selected: ' + obinfo

    BGL.glClearColor(0.4,0.5,0.8,1)
    BGL.glClear(BGL.GL_COLOR_BUFFER_BIT)
    BGL.glColor3f(1,1,1)
    x = y = 20
    w = 60
    h = 25
    BGL.glRasterPos2i(x, y + 9)
    Draw.Text(obinfo)
    Draw.PushButton('EXPORT', EVT_EXPORT, x+202, y, w, h,
      'Write track file(s).')
    y += 30
    w = 40
    h = 20
    pathmsg = 'Base dir where files will be saved.'
    if BUTPATH.val[-1] != bsys.sep: pathmsg  += ' And data filename head.'
    BUTPATH = Draw.String('path: ', EVT_PATH, x, y, 5*w, h, BUTPATH.val, 100,
      pathmsg)
    Draw.PushButton('browse...', EVT_BROWSE, x+202, y, 60, 20,
      'Open file selector (select any file in the chosen dir)') 
    y += 24
    w = 26
    h = 15
    BUTALL = Draw.Toggle('all', EVT_ALL, x, y, w, h, BUTALL.val,
      "Export all meshes and empties, not only selected ones.")
    x += 7
    BUTTRK = Draw.Toggle('trk', EVT_TRK, x+w, y, w, h, BUTTRK.val,
      "Write track (.ac) file.")
    BUTLOC = Draw.Toggle('loc', EVT_LOC, x+2*w, y, w, h, BUTLOC.val,
      "Write location (.loc) file.")
    BUTDRV = Draw.Toggle('drv', EVT_DRV, x+3*w, y, w, h, BUTDRV.val,
      "Write drive line (.drv) file.")
    x += 7 + 4*w
    w = 41
    Draw.PushButton('config', EVT_CONFIG, x, y, w, h,
      'Additional configurations (see help).')
    Draw.PushButton('help', EVT_HELP, x+w, y, w, h,
      'About, links and instructions.')
    Draw.PushButton('exit', EVT_EXIT, x+2*w+2, y, 60, h,
      'Exit from script (shortcuts: Esc, q).')
    x = 20
    y += 46
    BGL.glRasterPos2i(x, y)
    title = "TuxKart Track Exporter v%s" % VERSION
    tlen = Draw.Text(title)
    BGL.glBegin(BGL.GL_LINE_LOOP)
    BGL.glVertex2i(x+2+tlen, y+18)
    BGL.glVertex2i(x+2+tlen, y-10)
    BGL.glVertex2i(x-4, y-10)
    BGL.glVertex2i(x-4, y+18)
    BGL.glEnd()

def event(evt, val): # input event callback
  global HELPME, CONFIG

  if not val: return

  if HELPME or CONFIG:
    if evt == Draw.ESCKEY or evt == Draw.QKEY:
      HELPME = CONFIG = 0
      Draw.Redraw()
    return

  if evt == Draw.ESCKEY or evt == Draw.QKEY:
    update_RegistryInfo()
    Draw.Exit()
    return

def b_event(evt): # gui button events callback
  global AC_SKIP_DATA, AC_MIRCOL_AS_AMB, AC_MIRCOL_AS_EMIS, AC_ADD_DEFAULT_MAT
  global CONFIG, HELPTEXT, TK_ALL, TK_DRV, TK_LOC, TK_TRK, TK_PATH, TK_TUNE
  global HELPME, ARG, BUTPATH, BUTTRK, BUTDRV, BUTALL, BUTLOC, BUTTUNE
  global HTOG, HTOG_TRK, HTOG_DRV, HTOG_LOC, HTOG_INTRO

  if evt == EVT_EXPORT:
    fname = TK_PATH
    if bsys.exists(TK_PATH) == 2:
      fname += bsys.makename(Blender.Get('filename'), strip=1)
      FileSelector(fs_callback, "Export TuxKart track", fname)
    else: TK_Export(fname, 0)
  elif evt == EVT_ALL:
    TK_ALL = BUTALL.val
    if TK_ALL: ARG = 'all'
    else: ARG = 'sel'
  elif evt == EVT_DRV:
    TK_DRV = BUTDRV.val
  elif evt == EVT_LOC:
    TK_LOC = BUTLOC.val
  elif evt == EVT_TRK:
    TK_TRK = BUTTRK.val
  elif evt == EVT_PATH:
    TK_PATH = BUTPATH.val
    pathtype = bsys.exists(TK_PATH)
    if pathtype:
      if pathtype == 2 and TK_PATH[-1] != bsys.sep:
        TK_PATH += bsys.sep
      else: TKPATH = bsys.splitext(TK_PATH)[0]
      BUTPATH.val = TK_PATH
  elif evt == EVT_BROWSE:
    FileSelector(fs_cbPath, "Ok", TK_PATH)
  elif evt == EVT_TUNE:
    TK_TUNE = BUTTUNE.val
    if bsys.exists(TK_TUNE) != 1: # 1 is file
      Draw.PupMenu("ERROR%t|No such file.")
      TK_TUNE = ''
      BUTTUNE.val = TK_TUNE
  elif evt == EVT_TUNEBROWSE:
    FileSelector(fs_cbTune, "Select Song", TK_TUNE)
  elif evt == EVT_AC_DMAT:
    AC_ADD_DEFAULT_MAT = 1 - AC_ADD_DEFAULT_MAT
  elif evt == EVT_AC_SKIP:
    AC_SKIP_DATA = 1 - AC_SKIP_DATA
  elif evt == EVT_AC_MIR:
    AC_MIRCOL_AS_AMB = 1 - AC_MIRCOL_AS_AMB
  elif evt == EVT_AC_EMIS:
    AC_MIRCOL_AS_EMIS = 1 - AC_MIRCOL_AS_EMIS
  elif evt == EVT_AC_ALL:
    ARG = 'AC_all'
    fname = bsys.makename(ext=".ac")
    FileSelector(fs_callback, "Export AC3D", fname)
    return
  elif evt == EVT_AC_SEL:
    ARG = 'AC_sel'
    fname = bsys.makename(ext=".ac")
    FileSelector(fs_callback, "Export AC3D", fname)
    return
  elif evt == EVT_CONFIG:
    CONFIG = 1 - CONFIG
  elif evt == EVT_HELP:
    HELPME = 1 - HELPME
  elif evt == EVT_HELP_DRV:
    HELPTEXT = HELPTEXT_DRV
    HTOG = HTOG_DRV
  elif evt == EVT_HELP_LOC:
    HELPTEXT = HELPTEXT_LOC
    HTOG = HTOG_LOC
  elif evt == EVT_HELP_TRK:
    HELPTEXT = HELPTEXT_AC
    HTOG = HTOG_TRK
  elif evt == EVT_HELP_INTRO:
    HELPTEXT = HELPTEXT_INTRO
    HTOG = HTOG_INTRO
  elif evt == EVT_EXIT:
    update_RegistryInfo()
    Draw.Exit()
    return
  else: return
  Draw.Redraw()

def TK_Export(filename, onlytrack):
  global ARG

  starttime = bsys.time()

  # Here we check if given dir is TuxKart's one.
  # If so we save .ac in models/ and .loc and .drv in data/
  # else we save all in the given dir.
  datapath = modelspath = filename
  basename = bsys.basename(filename)
  dirname = bsys.dirname(filename)
  modelsdir = bsys.join(dirname, 'models')
  datadir = bsys.join(dirname, 'data')
  if bsys.exists (modelsdir) == 2:
    if bsys.exists(datadir) == 2:
      datapath = bsys.join(datadir, basename)
      modelspath = bsys.join(modelsdir, basename)

  scene = Blender.Scene.GetCurrent()
  objs = []

  # just the .ac export from the config screen:
  if onlytrack:
    if ARG == ('AC_sel'): objs = Blender.Object.GetSelected()
    else: objs = scene.getChildren()
    test = Track(objs, filename)
    endtime = bsys.time() - starttime
    Blender.Draw.PupMenu("OK|Data exported in %.2f seconds." % endtime)
    return

  if ARG == 'sel': objs = Blender.Object.GetSelected()
  else: objs = scene.getChildren()

  track = []
  thingies = []
  list = [o for o in objs if o.getType() == "Mesh" or o.getType() == "Empty"]
  for o in list:
    if o.getType() == "Empty": # empties whose names start with GR_
      if o.name.find("GR_") < 0: # are exported as AC groups info
        thingies.append(o)
    elif o.name == "DRV_LEFT":
      WriteLeftDriveLine(o, datapath)
    elif o.name == "DRV_RIGHT":
      WriteRightDriveLine(o, datapath)
    else: track.append(o)
  WriteLocation(thingies, datapath)
  test = Track(track, modelspath)
  endtime = bsys.time() - starttime
  Blender.Draw.PupMenu("OK|Data exported in %.2f seconds." % endtime)

# File Selector callbacks:
def fs_cbPath(path):
  global TK_PATH, BUTPATH
  TK_PATH = bsys.splitext(path)[0]
  BUTPATH.val = TK_PATH

def fs_cbTune(filename):
  global TK_TUNE, BUTTUNE
  TK_TUNE = filename
  BUTTUNE.val = TK_TUNE

def fs_callback(filename):
  global ARG

  #if hasattr(ARG, find):
  #  if ARG.find('AC_') == 0: onlytrack = 1
  #   else: onlytrack = 0
  onlytrack = 0

  TK_Export(filename, onlytrack)

# -- End of definitions

if not ARG or ARG == 'config': # not ARG if executed as loaded Blender Text
  Draw.Register(gui, event, b_event)
else:
  fname = Blender.sys.makename()
  FileSelector(fs_callback, "Export Tuxkart track", fname)
