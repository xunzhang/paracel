#! /usr/bin/env python
udct = {}
idct = {}
f = file('/mfs/user/wuhong/paracel/data/dtr/fm_train.data')
f_o = file('/mfs/user/wuhong/paracel/data/dtr/fm_train.data_compact', 'w')
for line in f:
    uid, iid, rating = line.strip().split(',')
    if uid not in udct:
        udct[uid] = len(udct)
    if iid not in idct:
        idct[iid] = len(idct)
    f_o.write('%s,%s,%s\n' % (udct[uid], idct[iid], rating))
f.close()
f_o.close()

f_o_ud = file('/mfs/user/wuhong/paracel/data/dtr/fm_train.data_compact_udct', 'w')
f_o_id = file('/mfs/user/wuhong/paracel/data/dtr/fm_train.data_compact_idct', 'w')
for k, v in udct.items():
    f_o_ud.write('%s\t%s\n' % (k, v))
for k, v in idct.items():
    f_o_id.write('%s\t%s\n' % (k, v))
f_o_ud.close()
f_o_id.close()
