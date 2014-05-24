from os import listdir

def is_numeric(x):
    try:
        float(x)
    except ValueError:
        return False
    return True
    
avg_lap_time = [0]*50
avg_position = [0]*50
avg_speed = [0]*50
avg_top = [0]*50
total_rescued = [0]*50
    
#for entry in ['sara','tux','elephpant','pidgin']:
    
tests = len(listdir('../batch'))-1    
for file in listdir('../batch'):
    if (file == '.DS_Store'):
        continue
    f = open('../batch/'+file,'r')
    """
    name_index = file.find('.')
    name = str(file[:name_index])
    first = file.find('.',name_index+1)
    numkarts = int(file[name_index+1:first])    
    second = file.find('.',first+1)
    laps = int(file[first+1:second])
    third = file.find('.',second+1)
    run = int(file[second+1:third])
    """
    mass = int(file[:file.find('.')])
    contents = f.readlines()
    contents = contents[2:contents.index("[debug  ] profile: \n")-1]
    content = [s for s in contents if "sara" in s]
    data = [float(x) for x in content[0].split() if is_numeric(x)]
    avg_lap_time[(mass-50)/10] += (data[2]/2)/5
    avg_position[(mass-50)/10] += data[1]/5
    avg_speed[(mass-50)/10] += data[3]/5
    avg_top[(mass-50)/10] += data[4]/5
    total_rescued[((mass-50)/10)] += data[7]
print("total rescue")
for i in range(len(avg_lap_time)):
    print(i*10+50, total_rescued[i])
        
        
    """avg_lap_time[i], avg_position[i], avg_speed[i], avg_top[i],"""
