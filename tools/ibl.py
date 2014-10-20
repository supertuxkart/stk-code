import Image as img
import numpy as np
import pylab as pl

n = 4

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
    "Xgrid/Ygrid/Zgrid must be normalised"
    Y00 = c00
    Y1minus1 = c1minus1 * Ygrid
    Y10 = c10 * Zgrid
    Y11 = c11 * Xgrid
    Y2minus2 = c2minus2 * Xgrid * Ygrid
    Y2minus1 = c2minus1 * Ygrid * Zgrid
    Y21= c21 * Xgrid * Zgrid
    Y20 = c20 * (3 * Zgrid * Zgrid - 1)
    Y22 = c22 * (Xgrid * Xgrid - Ygrid * Ygrid)
    return (Y00, Y1minus1, Y10, Y11, Y2minus2, Y2minus1, Y20, Y21, Y22)
    

Grid0, Grid1 = np.meshgrid(np.linspace(-1, 1, n), np.linspace(-1, 1, n))

FaceGrid = [(np.ones((n,n)), Grid0, Grid1), #GL_TEXTURE_CUBE_MAP_POSITIVE_X
    (-1 * np.ones((n,n)), Grid0, Grid1), #GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    (Grid0, np.ones((n,n)), Grid1), #GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    (Grid0, -1 * np.ones((n,n)), Grid1), #GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    (Grid0, Grid1, np.ones((n,n))), #GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    (Grid0, Grid1, -1 * np.ones((n,n)))] #GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    
res = []
for (Xd, Yd, Zd) in FaceGrid:
    res.append(computeYmlOnGrid(Xd, Yd, Zd))
    
print(res)

I = img.open("C:/Users/vljn_000/Documents/GitHub/stk-assets/textures/ants.png")
m = np.array(I)
print(type(m))
pl.imshow(m)
