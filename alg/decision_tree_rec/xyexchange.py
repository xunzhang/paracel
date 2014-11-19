#! /usr/bin/env python
f = file('/mfs/user/wuhong/paracel/data/dtr/netflix_train_compact')
f_o = file('/mfs/user/wuhong/paracel/data/dtr/netflix_train_compact2', 'w')
cnt = 0
for line in f:
    cnt += 1
    if cnt == 1:
        continue
    uid, iid, rating = line.strip('\n').split(',')
    f_o.write('%s,%s,%s\n' % (iid, uid, rating))
f.close()
f_o.close()
