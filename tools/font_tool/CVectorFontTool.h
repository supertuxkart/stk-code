
/*
	Vector font tool - Gaz Davidson December 2006-2009

	I noticed bitmap fonts were taking massive amounts of video memory at reasonable sizes,
	so I decided to make a vector font. I always wanted to try converting pixels to triangles...

	And I failed! This is a collection of the ugliest, bloated, most inneficient algorithms
	i've ever written, but its kinda working so I'm not changing it.
*/

#ifndef __VECTOR_FONT_TOOL_INCLUDED__
#define __VECTOR_FONT_TOOL_INCLUDED__

#include "irrlicht.h"
#include "CFontTool.h"
#include <assert.h>

using namespace irr;
using namespace video;

struct STriangleList
{
	core::array<core::vector2df>	positions;
	core::array<u16>		indexes;

	// for adding one triangle list to another,
	// these triangles share positions, but dont share triangles
	STriangleList& operator+=(STriangleList &other)
	{
		core::matrix4 m;
		core::array<s32> map;
		map.set_used(other.positions.size());

		for (u32 i=0; i<map.size(); ++i)
			map[i]=-1;

		for (u32 i=0; i<positions.size(); ++i)
			for (u32 j=0; j<map.size(); ++j)
				if ( positions[i] == other.positions[j] )
					map[j] = i;

		for (u32 i=0; i<map.size(); ++i)
			if (map[i] == -1)
			{
				positions.push_back(other.positions[i]);
				map[i] = positions.size()-1;
			}

		// add triangles
		for (u32 i=0; i<other.indexes.size(); ++i)
			indexes.push_back((u32)map[other.indexes[i]]);

		return *this;
	}

	// functions for building triangles for shapes,
	// each shape can't have duplicate triangles
	bool hasTriangle(core::vector2df a, core::vector2df b, core::vector2df c)
	{
		// make sure the triangle is wound correctly
		if (core::line2df(a,b).getPointOrientation(c) < 0)
			{ core::vector2df tmp=a; a=b; b=tmp; }

		u32 ia=0xffffffff, ib=0xffffffff, ic=0xffffffff;
		// Find each vertex
		for (u32 i=0; i < positions.size() && (ia==(u32)-1||ib==(u32)-1||ic==(u32)-1) ; ++i)
		{
			if (positions[i] == a)
				ia = i;
			if (positions[i] == b)
				ib = i;
			if (positions[i] == c)
				ic = i;
		}

		if (ia==0xffffffff)
		{
			return false;
		}
		if (ib==0xffffffff)
		{
			return false;
		}
		if (ic==0xffffffff)
		{
			return false;
		}

		for (u32 i=0; i<indexes.size(); i+=3)
			if ( (indexes[i] == ia && indexes[i+1] == ib && indexes[i+2] == ic) ||
				(indexes[i] == ic && indexes[i+1] == ia && indexes[i+2] == ib) ||
				(indexes[i] == ib && indexes[i+1] == ic && indexes[i+2] == ia) )
					return true;

		return false;
	}

	void add(core::vector2df a, core::vector2df b, core::vector2df c)
	{

		// make sure the triangle is wound correctly
		if (core::line2df(a,b).getPointOrientation(c) < 0)
		{
			core::vector2df tmp=a; a=b; b=tmp;
		}

		u32 ia=0xffffffff, ib=0xffffffff, ic=0xffffffff;
		// no duplicate vertex positions allowed...
		for (u32 i=0; i < positions.size() && (ia==-1||ib==-1||ic==-1) ; ++i)
		{
			if (positions[i] == a)
				ia = i;
			if (positions[i] == b)
				ib = i;
			if (positions[i] == c)
				ic = i;
		}
		bool found=true;
		if (ia==0xffffffff)
		{
			ia = positions.size();
			positions.push_back(a);
			found=false;
		}
		if (ib==0xffffffff)
		{
			ib = positions.size();
			positions.push_back(b);
			found=false;
		}
		if (ic==0xffffffff)
		{
			ic = positions.size();
			positions.push_back(c);
			found=false;
		}

		// no duplicate triangles allowed
		if (found)
		{
			found=false;
			for (u32 i=0; i<indexes.size(); i+=3)
			{
				if ( (indexes[i] == ia && indexes[i+1] == ib && indexes[i+2] == ic) ||
					(indexes[i] == ic && indexes[i+1] == ia && indexes[i+2] == ib) ||
					(indexes[i] == ib && indexes[i+1] == ic && indexes[i+2] == ia) )
				{
					found=true;
					break;
				}
			}
		}

		if (!found)
		{
			indexes.push_back(ia);
			indexes.push_back(ib);
			indexes.push_back(ic);
		}
	}
};

// finds groups of pixels and triangulates them
class CGroupFinder
{
public:
	CGroupFinder(bool *memory, s32 w, s32 h, IrrlichtDevice *dev):
		width(w), height(h), mem(memory), Device(dev)
	{
		refbuffer.set_used(w*h);
		for (u32 i=0; i<refbuffer.size(); ++i)
			refbuffer[i]=0;
		// find groups of pixels
		findGroups();
		removeGroups();

		// triangulate
		for (u32 i=0; i<groups.size(); ++i)
		{
			groups[i].triangulate();
		}
	}

	// contains a clockwise edge line
	struct SEdge
	{
		SEdge() : positions() { }

		core::array<core::position2di> positions;

		bool isMember(s32 x, s32 y)
		{
			for (u32 i=0; i<positions.size(); ++i)
				if (positions[i].X == x && positions[i].Y == y)
					return true;
			return false;
		}

		// reduces the number of points in the edge
		void reduce(s32 level=0)
		{
			// level 0- remove points on the same line
			for (u32 i=1; i < positions.size()-1; ++i)
			{
				// same point as the last one?! shouldnt happen, dunno why it does :|
				if (positions[i-1] == positions[i])
				{
					positions.erase(i--);
					continue;
				}

				// get headings
				core::vector2d<f32>	h1((f32)(positions[i-1].X - positions[i].X),(f32)(positions[i-1].Y - positions[i].Y)),
							h2((f32)(positions[i].X - positions[i+1].X),(f32)(positions[i].Y - positions[i+1].Y));
				h1.normalize();
				h2.normalize();

				if (h1==h2) // erase the current point
					positions.erase(i--);
			}

			// level 1- if point1 points at point3, we can skip point2
			// level 2+ allow a deviation of level-1

		}

	};

	// contains an array of lines for triangulation
	struct SLineList
	{
		core::array<core::line2df>	lines;
		SLineList() : lines() { }
		void addEdge(const SEdge &edge)
		{
			// adds lines to the buffer
			for (u32 i=1; i<edge.positions.size(); ++i)
				addLine(core::line2df((f32)edge.positions[i-1].X,	(f32)edge.positions[i-1].Y,
						(f32)edge.positions[i].X,	(f32)edge.positions[i].Y ));
		}
		void addLine( const core::line2df &line )
		{
			// no dupes!
			if (!hasLine(line))
				lines.push_back(line);
		}
		bool hasLine( const core::line2df &line )
		{
			for (u32 i=0; i<lines.size(); ++i)
				if (line == lines[i] || (line.start == lines[i].end && line.end == lines[i].start) )
					return true;
			return false;
		}

		bool crossesWith( core::line2df l, core::vector2df p)
		{
			// inside checks only work with clockwise triangles
			if (l.getPointOrientation(p) < 0)
				{ core::vector2df tmp=l.start; l.start=l.end; l.end=tmp; }

			// make the 3 triangle edges
			core::line2df &la=l, lb(l.end,p), lc(p,l.start);

			// test every line in the list
			for (u32 i=0; i<lines.size(); ++i)
			{
				core::line2df &l2 = lines[i];

				// the triangle isn't allowed to enclose any points
				// triangles are clockwise, so if to the right of all 3 lines, it's enclosed
				if (la.getPointOrientation(l2.start) > 0 &&
					lb.getPointOrientation(l2.start) > 0 &&
					lc.getPointOrientation(l2.start) > 0)
					return true;
				//if (la.getPointOrientation(l2.start) < 0 &&
				//	lb.getPointOrientation(l2.start) < 0 &&
				//	lc.getPointOrientation(l2.start) < 0)
				//	return true;

				core::vector2df out;
				//if (la.intersectWith(l2,out))
				//	if (out != la.start && out != la.end &&
				//		out != l2.start && out != l2.end)
				//		return true;
				if (lb.intersectWith(l2,out))
					if (!out.equals(lb.start) && !out.equals(lb.end) &&
						!out.equals(l2.start) && !out.equals(l2.end))
						return true;
				if (lc.intersectWith(l2,out))
					if (!out.equals(lc.start) && !out.equals(lc.end) &&
						!out.equals(l2.start) && !out.equals(l2.end))
						return true;

				// my shit intersection code only works with lines in certain directions :(
				if (l2.intersectWith(lb,out))
					if (!out.equals(lb.start) && !out.equals(lb.end) &&
						!out.equals(l2.start) && !out.equals(l2.end))
						return true;
				if (l2.intersectWith(lc,out))
					if (!out.equals(lc.start) && !out.equals(lc.end) &&
						!out.equals(l2.start) && !out.equals(l2.end))
						return true;


				if (lb.isPointOnLine(l2.start) && l2.start != lb.start && l2.start != lb.end)
					return true;
				if (lc.isPointOnLine(l2.start) && l2.start != lc.start && l2.start != lc.end)
					return true;

			}
			return false;
		}
	};

	// an area of adjacent pixels
	struct SPixelGroup
	{
		SPixelGroup(IrrlichtDevice *device) : triangles(), pixelWidth(0), pixelHeight(0),
			Device(device) {}

		core::array<core::position2di>	pixels;
		core::array<SEdge>		edges;
		STriangleList		triangles;
		core::array<SLineList>	ll;
		core::array<bool>		isMemberCache;
		s32			pixelWidth;
		s32			pixelHeight;
		IrrlichtDevice		*Device;

		void triangulate()
		{

			// find edges in this group
			makeEdges();

			// triangulate the group
			makeTriangles();

		}

		void drawTriangle( core::line2df line, core::vector2df point)
		{
			//const u32 endt = Device->getTimer()->getTime() + t;
			f32 scale = 5;


			//while(Device->getTimer()->getTime() < endt )
			//{
				Device->run();
				Device->getVideoDriver()->beginScene(true,true,video::SColor(0,0,0,0));
				for (u32 v=0;v<ll.size(); ++v)
				for (u32 h=0;h<ll[v].lines.size(); ++h)
				{
					core::line2df &currentline = ll[v].lines[h];
					core::position2di st = core::position2di((s32)(currentline.start.X*scale)+50, (s32)(currentline.start.Y*scale)+50);
					core::position2di en = core::position2di((s32)(currentline.end.X*scale)+50, (s32)(currentline.end.Y*scale)+50);

					Device->getVideoDriver()->draw2DLine(st,en, SColor(255,255,255,255));
				}
				// draw this triangle
				const core::position2di st((s32)(line.start.X*scale)+50, (s32)(line.start.Y*scale)+50);
				const core::position2di en((s32)(line.end.X*scale)+50, (s32)(line.end.Y*scale)+50);
				const core::position2di p((s32)(point.X*scale)+50, (s32)(point.Y*scale)+50);
				Device->getVideoDriver()->draw2DLine(st,en, SColor(255,255,0,0));
				Device->getVideoDriver()->draw2DLine(en,p, SColor(255,0,255,0));
				Device->getVideoDriver()->draw2DLine(p,st, SColor(255,0,0,255));

				Device->getVideoDriver()->endScene();
			//}
		}

		void makeTriangles()
		{
			// make lines from edges, because they're easier to deal with
			ll.clear();
			for (u32 i=0; i < edges.size(); ++i)
			{
				SLineList l;
				l.addEdge(edges[i]);
				ll.push_back(l);
			}
			// add an extra one for inside edges
			SLineList innerlines;
			ll.push_back(innerlines);

			// loop through each edge and make triangles
			for (u32 i=0; i<ll.size(); ++i)
			{
				// loop through each line in the edge
				for (u32 cl=0; cl<ll[i].lines.size(); ++cl)
				{

					core::line2df &currentLine = ll[i].lines[cl];
					f32 bestScore = -10.0f;
					s32 bestEdge = -1;
					s32 bestPoint = -1;
					// find the best scoring point to join to this line
					for (u32 k=0; k<ll.size(); ++k)
						for (u32 j=0; j< ll[k].lines.size(); ++j)
						{
							f32 score = 0.0f;
							core::vector2df point(ll[k].lines[j].start.X,
											ll[k].lines[j].start.Y);
							core::line2df line1(point,currentLine.start);
							core::line2df line2(currentLine.end,point);

							// can't be part of the current line
							if (point == currentLine.start || point == currentLine.end)
								continue;

							// must be to the right hand side (triangles are wound clockwise)
							// unless its part of the inside...
							f32 side1 = currentLine.getPointOrientation(point);
							f32 side2 = core::line2df(point,currentLine.start).getPointOrientation(currentLine.end);
							f32 side3 = core::line2df(currentLine.end,point).getPointOrientation(currentLine.start);
							if (i<ll.size()-1)
								if (side1 <= 0 || side2 <= 0 || side3 <=0)
									continue;

							// can't already have this triangle
							if (triangles.hasTriangle(currentLine.start,currentLine.end,point))
								continue;

							// must not cross any other lines or enclose any points
							bool itCrossed = false;
							for (u32 v=0; v<ll.size(); ++v)
								if (ll[v].crossesWith(currentLine, point))
								{
									itCrossed = true;
									break;
								}
							if (itCrossed)
								continue;


							// so, we like this triangle, but how much?
							// is it better than all the others?

							// we prefer points from other edges, unless its on the inside
							if (k==i && i != ll.size()-1)
								score = 1;
							else
								score = 2;

							// we prefer evenly shaped triangles

							// we prefer triangles with a large area

							// do we like this one more than the others?
							if (score>bestScore)
							{
								bestScore = score;
								bestEdge = k;
								bestPoint = j;
							}
						}
					// hopefully we found one
					if (bestEdge >= 0 && bestPoint >= 0 && bestScore >= 0.0f)
					{
						//assert(bestEdge >= 0 && bestPoint >= 0);
						//assert(bestScore >= 0.0f);

						core::vector2df point(ll[bestEdge].lines[bestPoint].start.X, ll[bestEdge].lines[bestPoint].start.Y);

						// add it to the triangles list
						triangles.add(currentLine.start, currentLine.end, point);

						// add inner lines to the line buffer, but only if they arent in others

						core::line2df la(point,currentLine.start);
						core::line2df lb(currentLine.end,point);

						bool found = false;
						for (u32 lineno=0;lineno<ll.size()-1; ++lineno)
							if (ll[lineno].hasLine(la))
								{ found=true; break; }
						if (!found)
							ll[ll.size()-1].addLine(la);

						for (u32 lineno=0;lineno<ll.size()-1; ++lineno)
							if (ll[lineno].hasLine(lb))
								{ found=true; break; }
						if (!found)
							ll[ll.size()-1].addLine(lb);

						//drawTriangle(currentLine, point);

					}

				}
			}
		}

		// finds the edges
		void makeEdges()
		{

			// speed it up
			refreshIsMemberCache();

			// clear the edges
			edges.clear();

			// loop through each pixel
			for (u32 i=0; i < pixels.size(); ++i)
			{
				core::position2di &p = pixels[i];
				s32 &x=p.X, &y=p.Y;
				bool ul = isMember(p.X-1,p.Y-1);
				bool u = isMember(p.X,p.Y-1);
				bool ur = isMember(p.X+1,p.Y-1);
				bool l = isMember(p.X-1,p.Y);
				bool r = isMember(p.X+1,p.Y);
				bool bl = isMember(p.X-1,p.Y+1);
				bool b = isMember(p.X,p.Y+1);
				bool br = isMember(p.X+1,p.Y+1);

				// walls already added?
				bool top=u, bottom=b, left=l, right=r;

				if (!(ul | u | ur | l | r | bl | b | br))
				{
					// lone square
					SEdge a;
					a.positions.push_back( core::position2di(x,y));
					a.positions.push_back( core::position2di(x+1,y));
					a.positions.push_back( core::position2di(x+1,y+1));
					a.positions.push_back( core::position2di(x,y+1));
					a.positions.push_back( core::position2di(x,y));
					edges.push_back(a);
					top=bottom=left=right=true;
				}
				else
				{
					if (!(ul|u|l) && (b&r) )
					{
						// upper outer diagonal "/"
						addToEdges(x,y+1,x+1,y);
						top=left=true;
					} else if ( !(u|ur|r) && (b&l) )
					{
						// upper outer diagonal "\"
						addToEdges(x,y,x+1,y+1);
						top=right=true;
					} else if ( !(l|bl|b) && (r&u) )
					{
						// lower outer diagonal "\"
						addToEdges(x+1,y+1,x,y);
						left=bottom=true;
					} else if ( !(r|br|b) && (l&u) )
					{
						// lower outer diagonal "/"
						addToEdges(x+1,y,x,y+1);
						right=bottom=true;
					}/* else if (!(b) && (l&bl) )
					{
						// upper inner diagonal "/"
						addToEdges(x+1,y+1,x,y+2);
						//bottom=true;
					} else if ( !(b) && (r&br) )
					{
						// upper inner diagonal "\"
						addToEdges(x+1,y+2,x,y+1);
						//bottom=true;
					} else if ( !(r) && (b&br) )
					{
						// lower inner diagonal "\"
						addToEdges(x+1,y,x+2,y+1);
						//right=true;
					} else if ( !(l) && (b&bl) )
					{
						// lower inner diagonal "/"
						addToEdges(x-1,y+1,x,y);
						//left=true;
					}*/

					// add flat edges
					if (!left	/*&& !( (u&ul) || (b&bl)) */)	addToEdges(x,y+1,x,y);
					if (!top	/*&& !( (l&ul) || (r&ur)) */)	addToEdges(x,y,x+1,y);
					if (!right	/*&& !( (u&ur) || (b&br)) */)	addToEdges(x+1,y,x+1,y+1);
					if (!bottom /*&& !( (l&bl) || (r&br)) */)	addToEdges(x+1,y+1,x,y+1);
				} // lone square
			} // for

			// reduce the number of points in each edge
			for (u32 i=0; i<edges.size(); ++i)
			{
				edges[i].reduce(1);

				// all edges should have at least 3 points
				assert(edges[i].positions.size() >= 3);

				// all edges should be closed
				assert(edges[i].positions[0] == edges[i].positions[edges[i].positions.size()-1] );
			}
		}

		// adds a line to the edges arrays
		void addToEdges(s32 x1, s32 y1, s32 x2, s32 y2)
		{
			bool found=false;
			// loop through each edge
			for (u32 i=0; i<edges.size(); ++i)
			{
				// if this line starts at the end of an edge
				if ( edges[i].positions[edges[i].positions.size()-1] == core::position2di(x1,y1))
				{
					// add it to the end
					edges[i].positions.push_back(core::position2di(x2,y2));
					found=true;
					break;
				}
				// if the line ends at the start of the edge
				if ( edges[i].positions[0]== core::position2di(x2,y2))
				{
					// add it to the front
					edges[i].positions.push_front(core::position2di(x1,y1));
					found=true;
					break;
				}
			}
			if (!found)
			{
				// we make a new edge
				SEdge n;
				n.positions.push_back(core::position2di(x1,y1));
				n.positions.push_back(core::position2di(x2,y2));
				edges.push_back(n);
			}

			joinEdges();
		}

		void joinEdges()
		{
			// touching edges are joined

			for (u32 i=0; i < edges.size(); ++i)
				for (u32 j=0; j < edges.size(); ++j)
			{
				if (i != j && edges[j].positions.size() && edges[i].positions.size())
				{
					if (edges[j].positions[0] == edges[i].positions[edges[i].positions.size()-1])
					{
						for (u32 k=0; k < edges[j].positions.size(); ++k)
							edges[i].positions.push_back(edges[j].positions[k]);
						edges[j].positions.clear();
					}
				}
			}

			// remove empty edges
			for (u32 i=0; i<edges.size(); ++i)
				if (edges[i].positions.size() == 0)
					edges.erase(i--);
		}

		// tells if this x,y position is a member of this group
		bool isMember(s32 x, s32 y)
		{
			//for (u32 i=0; i<pixels.size(); ++i)
			//	if (pixels[i].X == x && pixels[i].Y == y)
			//	return true;
			if (x>pixelWidth || y>pixelHeight || x<0 || y<0)
				return false;
			else
				return isMemberCache[pixelWidth*y + x];
		}

		void refreshIsMemberCache()
		{
			isMemberCache.clear();
			pixelWidth=0; pixelHeight=0;
			for (u32 i=0; i<pixels.size(); ++i)
			{
				if (pixels[i].X>pixelWidth) pixelWidth=pixels[i].X;
				if (pixels[i].Y>pixelHeight) pixelHeight=pixels[i].Y;
			}
			pixelWidth+=2; pixelHeight+=2;
			isMemberCache.set_used(pixelWidth*pixelHeight+1);
			for (u32 i=0; i<isMemberCache.size(); ++i)
				isMemberCache[i] = false;
			for (u32 i=0; i<pixels.size(); ++i)
				isMemberCache[pixelWidth*pixels[i].Y + pixels[i].X] = true;
		}
	};


	void drawEdges(IrrlichtDevice *device, u32 t, s32 scale)
	{
		const u32 stt = device->getTimer()->getTime();
		const u32 endt = stt + t;

		while(device->getTimer()->getTime() < endt )
		{
			const f32 phase = f32((device->getTimer()->getTime()-stt) % 500) / 500.0f;

			device->run();
			device->getVideoDriver()->beginScene(true,true,video::SColor(0,0,0,0));
			for (u32 g=0;g<groups.size(); ++g)
			for (u32 v=0;v<groups[g].edges.size(); ++v)
			for (u32 p=1;p<groups[g].edges[v].positions.size(); ++p)
			{
				core::position2di st = core::position2di(groups[g].edges[v].positions[p-1].X*scale+50, groups[g].edges[v].positions[p-1].Y*scale+50) ;
				core::position2di en = core::position2di(groups[g].edges[v].positions[p].X*scale+50, groups[g].edges[v].positions[p].Y*scale+50) ;
				core::position2di ep = en-st;
				ep = st + core::position2di((s32)(ep.X*phase), (s32)(ep.Y*phase));
				device->getVideoDriver()->draw2DLine(st,en);
				device->getVideoDriver()->draw2DLine(st,ep,video::SColor(255,255,0,0) );
			}
			device->getVideoDriver()->endScene();
		}
	}

	void drawTriangles(IrrlichtDevice *device, u32 t, s32 scale)
	{
		const u32 stt = device->getTimer()->getTime();
		const u32 endt = stt + t;

		while(device->getTimer()->getTime() < endt )
		{
			const f32 phase = f32((device->getTimer()->getTime()-stt) % 500) / 500.0f;

			device->run();
			device->getVideoDriver()->beginScene(true,true,video::SColor(0,0,0,0));
			for (u32 g=0;g<groups.size(); ++g)
			for (u32 v=0;v<groups[g].triangles.indexes.size()*phase; v+=3)
			{
				STriangleList &t = groups[g].triangles;
				core::position2di st((s32)(t.positions[t.indexes[v+0]].X*scale)+50,(s32)(t.positions[t.indexes[v+0]].Y*scale)+50);
				core::position2di en((s32)(t.positions[t.indexes[v+1]].X*scale)+50,(s32)(t.positions[t.indexes[v+1]].Y*scale)+50);
				device->getVideoDriver()->draw2DLine(st,en, SColor(255,255,0,0));
				st = core::position2di((s32)(t.positions[t.indexes[v+1]].X*scale)+50,(s32)(t.positions[t.indexes[v+1]].Y*scale)+50);
				en = core::position2di((s32)(t.positions[t.indexes[v+2]].X*scale)+50,(s32)(t.positions[t.indexes[v+2]].Y*scale)+50);
				device->getVideoDriver()->draw2DLine(st,en, SColor(255,0,255,0));
				st = core::position2di((s32)(t.positions[t.indexes[v+2]].X*scale)+50,(s32)(t.positions[t.indexes[v+2]].Y*scale)+50);
				en = core::position2di((s32)(t.positions[t.indexes[v+0]].X*scale)+50,(s32)(t.positions[t.indexes[v+0]].Y*scale)+50);
				device->getVideoDriver()->draw2DLine(st,en, SColor(255,0,0,255));
			}
			device->getVideoDriver()->endScene();
		}
	}

	void drawTriLines(IrrlichtDevice *device, u32 t, s32 scale)
	{
		const u32 endt = device->getTimer()->getTime() + t;

		while(device->getTimer()->getTime() < endt )
		{
			device->run();
			device->getVideoDriver()->beginScene(true,true,video::SColor(0,0,0,0));
			for (u32 g=0;g<groups.size(); ++g)
			for (u32 v=0;v<groups[g].ll.size()-1; ++v)
			for (u32 h=0;h<groups[g].ll[v].lines.size(); ++h)
			{
				core::line2df &currentline = groups[g].ll[v].lines[h];
				const core::position2di st((s32)(currentline.start.X*scale)+50, (s32)(currentline.start.Y*scale)+50);
				const core::position2di en((s32)(currentline.end.X*scale)+50, (s32)(currentline.end.Y*scale)+50);
				device->getVideoDriver()->draw2DLine(st,en, SColor(255,255,0,0));
			}

			device->getVideoDriver()->endScene();
		}
	}
	void drawTri3D(IrrlichtDevice *device, u32 t)
	{
		for (u32 g=0;g<groups.size(); ++g)
		{
			STriangleList &t = groups[g].triangles;
			core::array<S3DVertex> verts;
			verts.clear();
			for(u32 v=0; v< t.positions.size(); ++v)
			{
				verts.push_back(S3DVertex(
							-t.positions[v].X, -t.positions[v].Y, -100,
							0,0,1,SColor(255,255,255,255),0,0));
			}

			device->getVideoDriver()->drawIndexedTriangleList(verts.pointer(),verts.size(),t.indexes.pointer(), t.indexes.size()/3 );
		}
	}


	// process all pixels
	void findGroups()
	{
		for (int y=0; y<height; ++y)
			for (int x=0; x<width; ++x)
				processPixel(x,y);

	}

	// remove groups with no pixels
	void removeGroups()
	{
		for (u32 i=0; i<groups.size(); ++i)
			if (groups[i].pixels.size() == 0)
				groups.erase(i--);

		/*for (s32 y=0; y <height; ++y)
		{
			printf("\n");
			for (s32 x=0; x <width; ++x)
			{
				s32 i;
				for (i=0; i<groups.size(); ++i)
				{
					bool k = groups[i].isMember(x,y);
					if (k)
						break;
				}
				printf("%d",i);
			}
		}*/


	}

	// adds a pixel to its area, merging touching areas
	void processPixel(s32 x, s32 y)
	{
		// solid?
		if (getPixel(x,y))
		{
			s32 g=0, grp=0;

			bool found=false;
			if (x>0) // look one behind
			{
				grp = getRef(x-1,y);
				if (grp) found=true;
			}
			if (y>0) // look above
			{
				if (x>0) // top left
				{
					g = getRef(x-1,y-1);

					if (g)
					{
						if (found)
						{
							mergeGroups(grp, g);
						}
						else
						{
							grp = g;
							found = true;
						}
					}
				}

				if (x<width-1) // top right
				{
					g = getRef(x+1,y-1);

					if (g)
					{
						if (found)
						{
							mergeGroups(grp, g);
						}
						else
						{
							grp = g;
							found = true;
						}
					}
				}

				// top

				g = getRef(x,y-1);

				if (g)
				{
					if (found)
					{
						mergeGroups(grp, g);
					}
					else
					{
						grp = g;
						found = true;
					}
				}

			}

			// didn't find a group for this pixel, so we add one
			if (!found)
			{
				SPixelGroup p(Device);
				p.pixels.push_back(core::position2di(x,y));
				groups.push_back(p);
				groupRefs.push_back(groups.size());
				grp=groups.size();
			}
			else
			{
				groups[groupRefs[grp-1]-1].pixels.push_back(core::position2di(x,y));
			}
			setRef(x,y,groupRefs[grp-1]);
		}
	}

	bool& getPixel(s32 x, s32 y) { return mem[y*width +x]; }
	s32& getRef(s32 x, s32 y) { return refbuffer[y*width +x]; }
	void setRef(s32 x, s32 y, s32 g) { refbuffer[y*width +x] = g; }

	void mergeGroups(s32 g1, s32 g2)
	{
		if (g1==g2)
			return;
		// joins two groups together
		for (u32 i=0; i<groups[g2-1].pixels.size(); ++i)
			groups[g1-1].pixels.push_back(groups[g2-1].pixels[i]);
		groups[g2-1].pixels.clear();
		groupRefs[g2-1] = g1;
	}

	s32 width, height;
	core::array<SPixelGroup> groups;
	core::array<s32> groupRefs;
	core::array<s32> refbuffer;
	bool *mem;
	IrrlichtDevice *Device;
};

// creates a simple vector font from a bitmap from the font tool
class CVectorFontTool
{
public:
	CVectorFontTool(CFontTool *fonttool) :
		triangulator(0), FontTool(fonttool), 
		letterHeight(0), letterWidth(0), triangles()
	{
		core::map<wchar_t, u32>::Iterator it = FontTool->CharMap.getIterator();

		while(!it.atEnd())
		{
			CFontTool::SFontArea &fa = FontTool->Areas[(*it).getValue()];

			if (fa.rectangle.getWidth() > letterWidth)
				letterWidth = fa.rectangle.getWidth();
			if (fa.rectangle.getHeight() > letterHeight)
				letterHeight = fa.rectangle.getHeight();

			it++;
		}

		// number of verts is one more than number of pixels because it's a grid of squares
		letterWidth++;
		letterHeight++;

		// create image memory
		imagedata.set_used(letterWidth*letterHeight);

		// create vertex list, set position etc
		verts.set_used(letterWidth*letterHeight);
		for (s32 y=0; y<letterHeight; ++y)
		{
			for (s32 x=0; x<letterWidth; ++x)
			{
				S3DVertex &v = getVert(x,y);
				v.Pos = core::vector3df((f32)x,(f32)y,0.0f);
				v.TCoords.X = (f32)letterWidth / (f32)x;
				v.TCoords.Y = (f32)letterHeight / (f32)y;
				v.Normal = core::vector3df(0,0,-1);
				v.Color = SColor(255,255,255,255);
			}
		}
		// clear index list
		inds.clear();

		// create each char in the font...
		it = FontTool->CharMap.getIterator();
		while(!it.atEnd())
		{
			addChar((*it).getKey());
			it++;
		}
	}

	~CVectorFontTool()
	{
		if (triangulator)
			delete triangulator;
	}

	void addChar(wchar_t thischar)
	{
		const s32 area = FontTool->CharMap[thischar];
		const CFontTool::SFontArea &fa = FontTool->Areas[area];

		const s32 img = fa.sourceimage;
		const core::rect<s32>& r = fa.rectangle;

		// init image memory
		IImage *image = FontTool->currentImages[img];
		for (u32 i=0; i < imagedata.size(); ++i)
			imagedata[i] = false;
		for (s32 y=r.UpperLeftCorner.Y; y < r.LowerRightCorner.Y; ++y)
		{
			for (s32 x=r.UpperLeftCorner.X; x < r.LowerRightCorner.X ; ++x)
				if (image->getPixel(x,y).getBlue() > 0)
				{
					imagedata[letterWidth*(y-r.UpperLeftCorner.Y) +(x-r.UpperLeftCorner.X)] = true;
				}
		}

		// get shape areas
		triangulator = new CGroupFinder(imagedata.pointer(), letterWidth, letterHeight, FontTool->Device );

		wprintf(L"Created character '%c' in texture %d\n", thischar, img );

		//floodfill->drawEdges(FontTool->Device, 500, 3);
		//floodfill->drawTriangles(FontTool->Device, 500,30);
		//floodfill->drawTriLines(FontTool->Device, 200,3);

		/*
		if (area==32 && map == 0)
		{
			scene::ISceneManager *smgr = FontTool->Device->getSceneManager();
			smgr->addCameraSceneNodeFPS();
			while(FontTool->Device->run())
			{
				//floodfill->drawEdges(FontTool->Device, 100, 30);
				FontTool->Device->getVideoDriver()->beginScene(true, true, video::SColor(0,200,200,200));
				smgr->drawAll();
				floodfill->drawTri3D(FontTool->Device, 100);
				FontTool->Device->getVideoDriver()->endScene();
			}
		}*/

		u32 lastind = triangles.indexes.size();

		// loop through each shape and add it to the current character...
		for (u32 i=0; i < triangulator->groups.size(); ++i)
			triangles += triangulator->groups[i].triangles;

		// add character details
		charstarts.push_back(lastind);
		charlengths.push_back(triangles.indexes.size() - lastind);
		chars.push_back(thischar);
	}

	bool saveVectorFont(const c8 *filename, const c8 *formatname)
	{
		IrrlichtDevice *Device = FontTool->Device;

		if (triangles.indexes.size() == 0)
		{
			Device->getLogger()->log("No vector data to write, aborting.");
			return false;
		}

		core::stringc fn = filename;

		if (core::stringc(formatname) == core::stringc("xml"))
		{
			fn += ".xml";
			io::IXMLWriter *writer = FontTool->Device->getFileSystem()->createXMLWriter(fn.c_str());

			// header and line breaks
			writer->writeXMLHeader();
			writer->writeLineBreak();

			// write info header
			writer->writeElement(L"font", false, L"type", L"vector");
			writer->writeLineBreak();
			writer->writeLineBreak();

			// write each letter

			for (u32 n=0; n<chars.size(); ++n)
			{
				u32 i = FontTool->CharMap[chars[n]];
				CFontTool::SFontArea &fa = FontTool->Areas[i];
				wchar_t c[2];
				c[0] = chars[n];
				c[1] = L'\0';
				core::stringw area, under, over;
				area = core::stringw(fa.rectangle.LowerRightCorner.X-
						fa.rectangle.UpperLeftCorner.X);
				area += L", ";
				area += fa.rectangle.LowerRightCorner.Y-
						fa.rectangle.UpperLeftCorner.Y;


				core::array<core::stringw> names;
				core::array<core::stringw> values;
				names.clear();
				values.clear();
				// char
				names.push_back(core::stringw(L"c"));
				values.push_back(core::stringw(c));

				// width+height
				names.push_back(core::stringw(L"wh"));
				values.push_back(area);

				// start
				names.push_back(core::stringw(L"st"));
				values.push_back(core::stringw(charstarts[n]));
				// length
				names.push_back(core::stringw(L"len"));
				values.push_back(core::stringw(charlengths[n]));

				if (fa.underhang != 0)
				{
					under = core::stringw(fa.underhang);
					names.push_back(core::stringw(L"u"));
					values.push_back(under);
				}
				if (fa.overhang != 0)
				{
					over = core::stringw(fa.overhang);
					names.push_back(core::stringw(L"o"));
					values.push_back(over);
				}
				writer->writeElement(L"c", true, names, values);

				writer->writeLineBreak();
			}

			// write vertex data
			core::stringw data, count;
			data = L"";
			count = core::stringw(triangles.positions.size());
			for (u32 i=0; i<triangles.positions.size(); ++i)
			{
				if (i!=0)
					data += L", ";
				data += (s32)triangles.positions[i].X;
				data += L",";
				data += (s32)triangles.positions[i].Y;
			}
			writer->writeElement(L"Vertices", true, L"count", count.c_str(), L"data", data.c_str());
			writer->writeLineBreak();

			// write index list
			data = L"";
			count = core::stringw(triangles.indexes.size());
			for (u32 i=0; i<triangles.indexes.size(); i+=3)
			{
				if (i!=0)
					data += L", ";
				data += triangles.indexes[i+0];
				data += L",";
				data += triangles.indexes[i+1],
				data += L",";
				data += triangles.indexes[i+2];
			}

			writer->writeElement(L"Indices", true, L"count", count.c_str(), L"data", data.c_str());
			writer->writeLineBreak();

			writer->writeClosingTag(L"font");

			writer->drop();

			Device->getLogger()->log("Font saved.");
			return true;

		}
		else if (core::stringc(formatname) == core::stringc("bin"))
		{
			FontTool->Device->getLogger()->log("binary fonts not supported yet, sorry");
			return false;
		}
		else
		{
			FontTool->Device->getLogger()->log("unsupported file format, unable to save vector font");
			return false;
		}
	}

	S3DVertex& getVert(s32 x, s32 y) { return verts[letterWidth*y +x]; }

	core::array<S3DVertex>	verts;
	core::array<u16>		inds;
	core::array<bool>		imagedata;

	core::array<s32>		charstarts;	// start position of each char
	core::array<s32>		charlengths;	// index count
	core::array<wchar_t>		chars;		// letters

	CGroupFinder*		triangulator;
	CFontTool*		FontTool;

	s32			letterHeight;
	s32			letterWidth;

	STriangleList		triangles;
};

#endif // __VECTOR_FONT_TOOL_INCLUDED__

