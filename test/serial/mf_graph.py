#! /usr/bin/env python
#
# Matrix factorization with bias
#

import numpy as np

class mf():
    def __init__(self, k = 100, rounds = 10, alpha = 0.001, beta = 0.01, train_fn = '', pred_fn = '', output = ''):
        self.k = k
	self.rounds = rounds
	self.alpha = alpha
	self.beta = beta
	self.train_fn = train_fn
	self.pred_fn = pred_fn
	self.output = output
	self.usr_dct = {}
	self.item_dct = {}
	self.rating_graph = {}
	self.rating_sz = 0
	self.miu = 0.
	self.rmse = 0.
	self.p = None
	self.q = None
	self.usr_bias = None
	self.item_bias = None
   
    def load(self):
        f = open(self.train_fn)
	for line in f:
	    uid, iid, rating = line.strip('\n').split(',')
	    rating = float(rating)
	    if uid not in self.usr_dct:
	        self.usr_dct[uid] = uid
	    if iid not in self.item_dct:
	        self.item_dct[iid] = iid
	    self.rating_graph.setdefault(self.usr_dct[uid], []).append((self.item_dct[iid], rating))
	    self.rating_sz += 1
	    self.miu += rating
	self.miu /= self.rating_sz
	print self.miu
	f.close()
        
    def estimate(self, i, j):
        return self.miu + self.usr_bias[i] + self.item_bias[j] + np.dot(self.p[i], self.q[j])

    def cal_rmse(self):
        import math
	self.rmse = 0.
	for u_indx, pair in self.rating_graph.iteritems():
	    for i_indx, rating in pair:
		self.rmse += (rating - self.estimate(u_indx, i_indx)) ** 2
	return math.sqrt(self.rmse / self.rating_sz)
    
    def learning(self):
        #import time
        self.p = {}
	self.q = {}
	self.usr_bias = {}
	self.item_bias = {}
	for uid in self.usr_dct:
	    self.p[uid] = np.random.rand(self.k) * 0.1
	    self.usr_bias[uid] = np.random.rand() * 0.1
	for iid in self.item_dct:
	    self.q[iid] = np.random.rand(self.k) * 0.1
	    self.item_bias[iid] = np.random.rand() * 0.1
	# learning
	for rd in xrange(self.rounds):
	    #start = time.time()
	    for u_indx, pair in self.rating_graph.iteritems():
	        for i_indx, rating in pair:
		    e = rating - self.estimate(u_indx, i_indx) 
		    # compute delta
		    delta_p = self.alpha * (2 * e * self.q[i_indx] - self.beta * self.p[u_indx])
		    delta_q = self.alpha * (2 * e * self.p[u_indx] - self.beta * self.q[i_indx])
		    # update with delta
		    self.p[u_indx] += delta_p 
		    self.q[i_indx] += delta_q
		    self.usr_bias[u_indx] += self.alpha * (2 * e - self.beta * self.usr_bias[u_indx]) 
		    self.item_bias[i_indx] += self.alpha * (2 * e - self.beta * self.item_bias[i_indx])
    
    def solve(self):
        self.load()
	print 'load done'
	self.learning()
    
    def predict_rating(self):
        f1 = open(self.pred_fn)
	f2 = open(self.output, 'w')
	for line in f1:
	    uid, iid = line.strip('\n').split(',')
	    u_indx = self.usr_dct[uid]
	    i_indx = self.item_dct[iid]
	    pred_rating = self.estimate(u_indx, i_indx)
	    f2.write('%s,%s,%s\n' % (uid, iid, pred_rating))
	f1.close()
	f2.close()

if __name__ == '__main__':
    mf_solver = mf(k = 80, rounds = 1, alpha = 0.005, beta = 0.02, train_fn = '/mfs/user/wuhong/paracel/test/serial/training.csv', pred_fn = '/mfs/alg/Rec_Competition/predict.csv', output = '/mfs/user/wuhong/paracel/test/serial/mf_result')
    mf_solver.solve()
    print mf_solver.cal_rmse()
    #mf_solver.predict_rating()
