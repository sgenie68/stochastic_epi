#!/bin/bash -l
#SBATCH -J covid
#SBATCH -A ndhb03003          # Project Account
#SBATCH --time=10:20:00        # Walltime HH:MM:SS
#SBATCH --mem=8G              # Memory
#SBATCH --cpus-per-task=4 --ntasks=1
#SBATCH packjob
#SBATCH --cpus-per-task=1 --ntasks=7


module load OpenMPI
module load libconfig
srun ./model -f virus.cfg -e 150 >days.csv
