path = '../data/cosmic_ray'
filename1 = 'run00188182_20191007_decode'
filename2 = 'run00188183_20191008_decode'
# filename1 = 'test1'
# filename2 = 'test2'

Inputfile1 = path + '/data_gen/' + filename1 + '.txt'
Inputfile2 = path + '/data_gen/' + filename2 + '.txt'

with open(Inputfile1, 'r') as copyfile:
    with open(Inputfile2, 'a') as pastefile:
        pastefile.write('\n')
        for line in copyfile:
            pastefile.write(line)