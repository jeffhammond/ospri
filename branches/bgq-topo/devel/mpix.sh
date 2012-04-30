#!/bin/bash -x
/bgsys/drivers/ppcfloor/bin/runjob --mapping TCEBAD --block ${COBALT_PARTNAME} --np 256 --ranks-per-node 2 --cwd /veas_home/jhammond/OSPRI/branches/bgq-topo/devel --envs BG_SHAREDMEMSIZE=32 --envs COBALT_JOBID=${COBALT_JOBID} --verbose 4 : /veas_home/jhammond/OSPRI/branches/bgq-topo/devel/mpix.x

