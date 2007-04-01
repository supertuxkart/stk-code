#!BPY

""" Registration info for Blender menus:
Name: 'Super Tux Kart Driveline'
Blender: 233
Group: 'Import'
Tip: 'Load Super Tux Kart drivelines.'
"""

# --------------------------------------------------------------------------
# ***** BEGIN GPL LICENSE BLOCK *****
#
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

import Blender

def import_driveline(path):
	Blender.Window.WaitCursor(1)
	name = path.split('\\')[-1].split('/')[-1]
 	mesh = Blender.NMesh.New( name ) # create a new mesh

 	file = open(path, 'r')
	i = 0

	v1 = Blender.NMesh.Vert(0.0,0.0,0.0)
	for line in file:
		words = line.split(',')

		if len(words) == 0:
			continue

		x = float(words[0])
		y = float(words[1])
		z = float(words[2])

		v = Blender.NMesh.Vert( x, y, z )
		mesh.verts.append(v)
		v.sel = 1

		if len(mesh.verts) >= 2:
			mesh.addEdge(mesh.verts[-2], mesh.verts[-1])
	
	mesh.addEdge(mesh.verts[0], mesh.verts[-1])

	# link the mesh to a new object
	ob = Blender.Object.New('Mesh', name)
	ob.link(mesh) # tell the object to use the mesh we just made
	scn = Blender.Scene.GetCurrent()
	for o in scn.getChildren():
		o.sel = 0
	scn.link(ob) # link the object to the current scene
	ob.sel= 1
	ob.Layers = scn.Layers
	Blender.Window.WaitCursor(0)
	Blender.Window.RedrawAll()

Blender.Window.FileSelector(import_driveline, 'Import')

