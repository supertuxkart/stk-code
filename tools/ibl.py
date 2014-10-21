import Image as img
import numpy as np
import pylab as pl

n = 3

# constant factor of Ylm
c00 = 0.282095
c1minus1 = 0.488603
c10 = 0.488603
c11 = 0.488603
c2minus2 = 1.092548
c2minus1 = 1.092548
c21 = 1.092548
c20 = 0.315392
c22 = 0.546274

def computeYmlOnGrid(Xgrid, Ygrid, Zgrid):
    "compute Yml from Y00 to Y22 on Xgrid/Ygrid/Zgrid"
    norm = np.sqrt(Xgrid * Xgrid + Ygrid * Ygrid + Zgrid * Zgrid)
    Xg = Xgrid / norm
    Yg = Ygrid / norm
    Zg = Zgrid / norm
    Y00 = c00
    Y1minus1 = c1minus1 * Yg
    Y10 = c10 * Zg
    Y11 = c11 * Xg
    Y2minus2 = c2minus2 * Xg * Yg
    Y2minus1 = c2minus1 * Yg * Zg
    Y21= c21 * Xg * Zg
    Y20 = c20 * (3 * Zg * Zg - 1)
    Y22 = c22 * (Xg * Xg - Yg * Yg)
    return (Y00, Y1minus1, Y10, Y11, Y2minus2, Y2minus1, Y20, Y21, Y22)
    

GridI, GridJ = np.meshgrid(np.linspace(-1, 1, n), np.linspace(-1, 1, n))

FaceGrid = [(np.ones((n,n)), -GridI, -GridJ), #GL_TEXTURE_CUBE_MAP_POSITIVE_X
    (-1 * np.ones((n,n)), -GridI, GridJ), #GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    (GridJ, np.ones((n,n)), GridI), #GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    (GridJ, -1 * np.ones((n,n)), -GridI), #GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    (GridJ, GridI, np.ones((n,n))), #GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    (GridJ, -GridI, -1 * np.ones((n,n)))] #GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    
res = []
for (Xd, Yd, Zd) in FaceGrid:
    res.append(computeYmlOnGrid(Xd, Yd, Zd))
    
    
# From http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/
def areaToPoint(x, y):
    return np.arctan2(x * y, np.sqrt(x * x + y * y + 1))
def getSolidAngleGrid(Xgrid, Ygrid):
    "Compute solid angles using Xgrid/Ygrid/Zgrid texel position"
    result = np.zeros((n,n))
    for i in range(n):
        for j in range(n):
            x = Xgrid[i,j]
            y = Ygrid[i,j]
            x0 = x - (1. / n)
            x1 = x + (1. / n)
            y0 = y - (1. / n)
            y1 = y + (1. / n)
            result[i,j] = areaToPoint(x0,y0) - areaToPoint(x1, y0) - areaToPoint(x0, y1) + areaToPoint(x1, y1)
    return result
    
print(getSolidAngleGrid(GridI, GridJ))

#I = img.open("C:/Users/vljn_000/Documents/GitHub/stk-assets/textures/ants.png")
#m = np.array(I)
#print(type(m))
#pl.imshow(m)
