gcc -g -fopenmp par_pi_1.c -o par_pi_1
gcc -g -fopenmp par_pi_2.c -o par_pi_2
gcc -g -fopenmp par_pi_3.c -o par_pi_3
gcc -g -fopenmp par_pi_4.c -o par_pi_4

./par_pi_1
./par_pi_2
./par_pi_3
./par_pi_4
g++ -g -fopenmp PSRS.cpp -o PSRS
./PSRS 3 27 15 46 48 93 39 6 72 91 14 36 69 40 89 61 97 12 21 54 53 97 84 58 32 27 33 72 20