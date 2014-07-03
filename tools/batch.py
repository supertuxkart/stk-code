from matplotlib import pyplot
from os import listdir


def is_numeric(x):
    try:
        float(x)
    except ValueError:
        return False
    return True
    
    
avg_lap_time = {}
avg_pos = {}
avg_speed = {}
avg_top = {}
total_rescued = {}
        
tests = len(listdir('../../batch'))-1    
for file in listdir('../../batch'):
    if (file == '.DS_Store'):
        continue
    f = open('../../batch/'+file,'r')
    
    
    '''
    name_index = file.find('.')
    kart_name = str(file[:name_index])
    first = file.find('.',name_index+1)
    track_name = file[name_index+1:first]
    second = file.find('.',first+1)
    run = int(file[first+1:second])
    '''
    track_name = "snowmountain"
    kart_names = ["gnu", "sara", "tux", "elephpant"]
    
    if track_name == "snowmountain":
        contents = f.readlines()
        '''
        contents = contents[2:contents.index("[debug  ] profile: \n")-1]
        content = [s for s in contents if kart_name in s]
        data = [float(x) for x in content[0].split() if is_numeric(x)]
        if kart_name not in avg_lap_time:
            avg_lap_time[kart_name] = []
            avg_pos[kart_name] = []
            avg_speed[kart_name] = []
            avg_top[kart_name] = []
            total_rescued[kart_name] = []

            avg_lap_time[kart_name].append(data[2]/4)
            avg_pos[kart_name].append(data[1])
            avg_speed[kart_name].append(data[3])
            avg_top[kart_name].append(data[4])
            total_rescued[kart_name].append(data[7])
        '''
        
        contents = contents[2:6] #TODO check if all is in here
        for kart in kart_names:
            content = [s for s in contents if kart in s]
            data = [float(x) for x in content[0].split() if is_numeric(x)]
            if kart not in avg_lap_time:
                avg_lap_time[kart] = []
                avg_pos[kart] = []
                avg_speed[kart] = []
                avg_top[kart] = []
                total_rescued[kart] = []
    
            avg_lap_time[kart].append(data[2]/4)
            avg_pos[kart].append(data[1])
            avg_speed[kart].append(data[3])
            avg_top[kart].append(data[4])
            total_rescued[kart].append(data[7])

tests = len(avg_lap_time["gnu"])        
print total_rescued


for kart in kart_names:
    print "rescues for ", kart , ": ", sum(total_rescued[kart])/tests
    print "avg_lap_time for " , kart , ": " , sum(avg_lap_time[kart])/tests
    print "avg_pos for " , kart , ": " , sum(avg_pos[kart])/tests
    print "avg_speed for " , kart , ": " , sum(avg_speed[kart])/tests
    print "avg_top for " , kart , ": " , sum(avg_top[kart])/tests
    

pyplot.subplot(2,2,1)
pyplot.plot(list(xrange(tests)),avg_pos["gnu"], "b-")
pyplot.xlabel("tests")
pyplot.ylabel("gnu")
pyplot.subplot(2,2,2)
pyplot.plot(list(xrange(tests)),avg_pos["sara"], "r-")
pyplot.xlabel("tests")
pyplot.ylabel("sara")
pyplot.subplot(2,2,3)
pyplot.plot(list(xrange(tests)),avg_pos["elephpant"], "y-")
pyplot.xlabel("tests")
pyplot.ylabel("elephpant")
pyplot.subplot(2,2,4)
pyplot.plot(list(xrange(tests)),avg_pos["tux"], "g-")
pyplot.xlabel("tests")
pyplot.ylabel("tux")

pyplot.show()
