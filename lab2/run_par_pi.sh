#!/bin/sh
mpicxx -o par_pi par_pi.cpp
mpirun -np 3 ./par_pi