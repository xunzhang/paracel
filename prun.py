#! /usr/bin/env python 
#
# Copyright (c) 2014, Douban Inc. 
#   All rights reserved. 
#
# Distributed under the BSD License. Check out the LICENSE file for full text.
#
# Paracel - A distributed optimization framework with parameter server.
#
# Downloading
#   git clone http://code.dapps.douban.com/wuhong/paracel.git
#
# Authors: Hong Wu <xunzhangthu@gmail.com>
#
#
# usage: paracelrun --snum 1 --wnum 1 --cfg ./alg/sgd/sgd_cfg.json --method local
# 		 paracelrun --snum 1 --wnum 1 --cfg ./alg/sgd/sgd_cfg.json --method mesos --ppn 1 --mem_limit 1000
#		 paracelrun --snum 1 --wnum 1 --cfg ./alg/sgd/sgd_cfg.json --method mpi --hostfile hostfile
#

try:
    from optparse import OptionParser
except:
    print 'optparse module required'
    exit(0)

import os
import json
import socket
import random
import subprocess

import logging
logging.basicConfig(filename='paracelrun_log', format = '%(asctime)s : %(levelname)s : %(message)s', level = logging.INFO)
logger = logging.getLogger(__name__)

def paracelrun_cpp_proxy(nsrv, initport):
	from subprocess import Popen, PIPE
	cmd_lst = ['./local/bin/paracelrun_cpp_proxy --nsrv', str(nsrv), '--init_port', str(initport)]
	cmd = ' '.join(cmd_lst)
	logger.info(cmd)
	p = Popen(cmd.split(), stdin = PIPE, stdout = PIPE)
	return p.stdout.readline()


def get_free_port():
	import os
	import random 
	def is_avaliable(port):
		import os
		cmd = 'netstat -tuln | grep LISTEN | cut -f 2 -d :'
		tmp = os.popen(cmd)
		content = tmp.read()
		content = content.strip('\n').split('0.0.0.0')
		plst = [item.strip('\n').strip(' ') for item in content]
		while '' in plst:
			plst.remove('')
		plst = [int(item) for item in plst]
		tmp.close()
		if port in plst:
			return False
		else:
			return True
	port = random.randint(10000, 65535)
	while not is_avaliable(port):
		port = random.randint(10000, 65535)
	return port

if __name__ == '__main__':
	optpar = OptionParser()
	optpar.add_option('-p', '--snum', default = 1,
					action = 'store', type = 'int', dest = 'parasrv_num', 
					help = 'number of parameter servers')
	optpar.add_option('-w', '--wnum', default = 1,
					action = 'store', type = 'int', dest = 'worker_num', 
					help = 'number of workers for learning')
	optpar.add_option('-c', '--cfg_file', 
					action = 'store', type = 'string', dest = 'config', 
					help = 'config file in json fmt, for alg usage')
	optpar.add_option('-m', '--method', default = 'local', 
					action = 'store', type = 'string', dest = 'method', 
					help = 'running method', metavar = 'local | mesos | mpi')
	optpar.add_option('--ppn', default = 1, 
					action = 'store', type = 'int', dest = 'ppn',
					help = 'procs number per node - mesos')
	optpar.add_option('--mem_limit', default = 200,
					action = 'store', type = 'int', dest = 'mem_limit',
					help = 'memory size of each task - mesos')
	optpar.add_option('--hostfile', 
					action = 'store', type = 'string', dest = 'hostfile', 
					help = 'hostfile for mpirun - mpi')
	(options, args) = optpar.parse_args()
    
	nsrv = 1
	nworker = 1
	if options.parasrv_num and options.worker_num: 
		nsrv = options.parasrv_num
	nworker = options.worker_num
    
	if options.method == 'mesos':
		starter = 'mrun -m ' + str(options.mem_limit) + ' -p ' + str(options.ppn) + ' -n'
	elif options.method == 'mpi':
		if options.hostfile:
			starter = 'mpirun --hostfile ' + options.hostfile + ' -n'
		else:
			starter = 'mpirun --hostfile ~/.mpi/large.18 -n'
	else:
		starter = 'mpirun -n'
    
	#initport = random.randint(30000, 65000)
	#initport = get_free_port()
	initport = 11777

	start_parasrv_cmd_lst = [starter, str(nsrv), './local/bin/start_server --start_host', socket.gethostname(), ' --init_port', str(initport)]
	start_parasrv_cmd = ' '.join(start_parasrv_cmd_lst)
	logger.info(start_parasrv_cmd)
	procs = subprocess.Popen(start_parasrv_cmd, shell = True, preexec_fn = os.setpgrp)

	try:
		serverinfo = paracelrun_cpp_proxy(nsrv, initport)
		entry_cmd = ''
		if args:
			entry_cmd = ' '.join(args)
		alg_cmd_lst = [starter, str(nworker), entry_cmd, '--server_info', serverinfo, '--cfg_file', options.config]
		alg_cmd = ' '.join(alg_cmd_lst)
		logger.info(alg_cmd)
		os.system(alg_cmd)
		os.killpg(procs.pid, 9)
	except:
		os.killpg(procs.pid, 9)
