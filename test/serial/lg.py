from math import e
from array import array
import numpy as np

def h(x,theta):
    tmp = np.dot(x, theta)
    tmp2 = 1. / (1. + e ** tmp)
    return e ** (tmp) * tmp2

def log_reg_regularized_sgd(x, y, alpha, beta = 0.1, max_iter = 100, debug = False):
    if debug: 
    	err = array('f', [])
    m,n = x.shape
    theta = np.random.random(n)
    z = np.arange(m)
    for t in xrange(max_iter):
        z = np.random.permutation(z)
	opt = 2. * alpha * beta
        for i in z:
	    theta = theta + alpha * (y[i] - h(x[i], theta)) * x[i] - opt * theta
	    #theta = theta + alpha * (y[i] - h(x[i], theta)) * x[i] - beta * 2. * alpha * theta
            if debug: 
	    	err.append(sum([(y[i] - h(x[i], theta)) ** 2 for i in range(m)]))
    if debug:
        return theta,err
    return theta

def log_reg_regularized_bgd(x, y, alpha, beta = 0.1, max_iter = 100, debug = False):
    if debug: 
    	err = array('f', [])
    m,n = x.shape
    theta = np.random.random(n)
    z = np.arange(m)
    for t in xrange(max_iter):
        z = np.random.permutation(z)
	opt = 2. * alpha * beta
	delta = [0 for i in xrange(n)]
        for i in z:
	    delta += alpha * (y[i] - h(x[i], theta)) * x[i] - opt * theta
	    #theta = theta + alpha * (y[i] - h(x[i], theta)) * x[i] - beta * 2. * alpha * theta
            if debug: 
	    	err.append(sum([(y[i] - h(x[i], theta)) ** 2 for i in range(m)]))
        theta += delta
    if debug:
        return theta,err
    return theta

def load(f):
    x = []
    y = []
    for line in f:
        arr = [float(i) for i in line.strip('\n').split(',')]
	x.append(arr[0:-1])
	y.append(arr[-1])
    x = np.array(x)
    x = np.hstack( (np.ones( (x.shape[0], 1.) ), x) )
    y = np.array(y)
    return x, y

def cal_loss(theta, x, y):
    loss = 0.
    for i in xrange(len(y)):
        loss += (h(x[i], theta) - y[i]) ** 2
    return loss / len(y)

if __name__ == '__main__':
    f = file('/mfs/user/wuhong/paracel/data/classification/train_000.csv')
    x, y = load(f)
    f.close()

    theta = log_reg_regularized_sgd(x, y, 0.001, 0.01, max_iter = 100, debug = False)
    print theta

    #theta, err = log_reg_regularized_sgd(x, y, 0.001, 0.1, max_iter = 1, debug = True)
    #print err

    f2 = file('/mfs/user/wuhong/paracel/data/classification/test_000.csv')
    x_t, y_t = load(f2)
    f.close()
    print 'mean loss', cal_loss(theta, x_t, y_t)
