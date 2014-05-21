from os import listdir
for file in listdir('../batch'):
    f = open('../batch/'+file,'r')
    for c in f.readlines():
        print(c)
    
