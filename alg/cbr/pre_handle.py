#! /bin/env python
from datetime import *

# uid, iid, rating, date 
def fmt(fn_in, fn_out):
    fin = file(fn_in)
    fout = file(fn_out, 'wb')
    for line in fin:
        lst = line.strip('\n').split(',')
        fout.write('%s,%s,%s\n' % (lst[0], lst[1], lst[2]))
    fin.close()
    fout.close()

# uid, iid, date, rating
def split_by_usr(fn_in, fn_out1, fn_out2, nbar = 0.7):
    fin = file(fn_in)
    fout_p1 = file(fn_out1, 'wb')
    fout_p2 = file(fn_out2, 'wb')
    dct = {}
    # init dict
    for line in fin:
        lst = line.strip('\n').split(',')
        if dct.get(lst[0]):
            dct[lst[0]].append((lst[1], lst[2], lst[3]))
        else:
            dct[lst[0]] = [(lst[1], lst[2], lst[3])]
   
    # split by nbar
    for k, v in dct.items():
        th = long(len(v) * nbar)
        for vv in v[:th]:
            fout_p1.write('%s,%s,%s,%s\n' % (k, vv[0], vv[1], vv[2]))
        for vv in v[th:]:
            fout_p2.write('%s,%s,%s,%s\n' % (k, vv[0], vv[1], vv[2]))
    fin.close()
    fout_p1.close()
    fout_p2.close()


# uid, iid, ... 
def split_by_item(fn_in, fn_out1, fn_out2, nbar = 0.7):
    fin = file(fn_in)
    fout_p1 = file(fn_out1, 'wb')
    fout_p2 = file(fn_out2, 'wb')
    dct = {}
    # init dict
    for line in fin:
        lst = line.strip('\n').split(' ')
        if dct.get(lst[1]):
            dct[lst[1]].append((lst[0], lst[2], lst[3]))
        else:
            dct[lst[1]] = [(lst[0], lst[2], lst[3])]
   
    # split by nbar
    for k, v in dct.items():
        th = long(len(v) * nbar)
        for vv in v[:th]:
            fout_p1.write('%s,%s,%s,%s\n' % (vv[0], k, vv[1], vv[2]))
        for vv in v[th:]:
            fout_p2.write('%s,%s,%s,%s\n' % (vv[0], k, vv[1], vv[2]))
    fin.close()
    fout_p1.close()
    fout_p2.close()

# uid, iid, rating, date
def split_by_date(fn_in, fn_out1, fn_out2, timebar = date(2005, 10, 1)):
    fin = file(fn_in)
    fout_p1 = file(fn_out1, 'wb')
    fout_p2 = file(fn_out2, 'wb')
    for line in fin:
        lst = line.strip('\n').split(',')
        dateinfo = [int(i) for i in lst[3].split('-')]
        timestamp = date(dateinfo[0], dateinfo[1], dateinfo[2])
        if timestamp < timebar:
            fout_p1.write(line)
        else:
            fout_p2.write(line)
    fin.close()
    fout_p1.close()
    fout_p2.close()

def sort_by_date(fn_in, fn_out):
    container = []
    fin = file(fn_in)
    fout = file(fn_out, 'wb')
    for line in fin:
        lst = line.strip('\n').split(',')
        dateinfo = [int(i) for i in lst[2].split('-')]
        timestamp = date(dateinfo[0], dateinfo[1], dateinfo[2])
        container.append((timestamp, lst[0], lst[1], lst[3]))
    for val in sorted(container, key = lambda v : v[0]):
        fout.write('%s,%s,%s,%s\n' % (val[1], val[2], val[3], str(val[0])))
    fin.close()
    fout.close()

# uid, iid, ...
def meta(fn):
    rating_sum = 0.
    udct = {}
    idct = {}
    fin = file(fn)
    for line in fin:
        lst = line.strip('\n').split(',')
        udct[lst[0]] = 1
        idct[lst[1]] = 1
    print 'user number is: ', len(udct)
    print 'item number is: ', len(idct)
    fin.close()

if __name__ == '__main__':
    #split_by_item('/mfs/user/wuhong/paracel/data/netflix/train/train', 
    #              '/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part1', 
    #              '/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part2')
    #sort_by_date('/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part2',
    #             '/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part2_sorted')
    #fmt('/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part2_sorted',
    #    '/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part2_sorted_fmt')
    meta('/mfs/user/wuhong/paracel/data/netflix/train_handled/train_split_part2_sorted_fmt')
