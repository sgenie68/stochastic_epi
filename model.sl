#!/bin/bash -l
#SBATCH -J covid
#SBATCH -A ndhb03003          # Project Account
#SBATCH --time=10:20:00        # Walltime HH:MM:SS
#SBATCH --mem=8G              # Memory
#SBATCH --ntasks=8            # number of tasks

module load OpenMPI
module load libconfig
srun ./model -f virus.cfg >days.csv
