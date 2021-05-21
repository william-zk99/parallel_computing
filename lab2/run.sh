#!/bin/sh
mpicxx -o PSRS PSRS.cpp
mpirun -np 4 ./PSRS