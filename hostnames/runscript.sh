#!/bin/bash
#SBATCH -n 128
#SBATCH -N 128
#SBATCH --time=00:05:00 # walltime, abbreviated by -t

mpirun -np 128 bash ./gethostname.sh


