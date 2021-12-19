#include "amoebasat.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
using namespace std;
unsigned state[1];
one_bit_t x[N_VARIABLE+1];
z_t Z_in[N_VARIABLE+1][2];

f_t f1[N_CLAUSE][3]={
2, 3, 1,
3, 4, 2,
3, 4, 2,
4, 1, 3,
4, 1, 3,
1, 2, 4,
1, 2, 4,
2, 3, 1,
2, 3, 1};
one_bit_t f1_sign[N_CLAUSE][3]={
0, 1, 0,
0, 1, 1,
0, 1, 0,
0, 1, 1,
0, 1, 0,
0, 1, 1,
0, 1, 0,
0, 1, 1,
0, 0, 0};

one_bit_t satisfiable(f_t inter[N_CLAUSE][N_LITERAL], one_bit_t inter_sign[N_CLAUSE][N_LITERAL], one_bit_t x[N_VARIABLE+1]){
	int clause_id;
	one_bit_t f_val = 1;
	Loop_update_f:for(clause_id=0; clause_id<N_CLAUSE; clause_id++){
		one_bit_t c1, c2, c3;
		c1 = x[ inter[clause_id][0] ] ^ inter_sign[clause_id][0];
		c2 = x[ inter[clause_id][1] ] ^ inter_sign[clause_id][1];
		c3 = x[ inter[clause_id][2] ] ^ inter_sign[clause_id][2];
		f_val = c1|c2|c3;
		if(!f_val) {
			cout << "unsatisfied clause " << clause_id+1 << " " << c1 << " " << c2 << " " << c3 << "\n";
			break;
		}
	}
	return f_val;
}
unsigned xorshift32(unsigned state[1])
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	unsigned z0= state[0];
	z0 ^= z0 << 13;
	z0 ^= z0 >> 17;
	z0 ^= z0 << 5;
	state[0] = z0;
	return z0;
}

void init(int seed, z_t Z_in[N_VARIABLE+1][2]){
    int i, j;
    state[0]=seed;
    Loop_init:
    for(i=1;i<=N_VARIABLE;i++){
    	Z_in[i][0] = xorshift32(state)>>1; //cout << "Z_in[][0]=" <<Z_in[i][0] << "\n";
    	Z_in[i][1] = xorshift32(state)>>1; //cout << "Z_in[][1]=" <<Z_in[i][1] << "\n";
    }
}

int main() {
	int i;
    int NStep = 0;
    int NStep_avg = 0;
    one_bit_t sat = 0;
    srand(time(NULL));

    for(i=0;i<1;i++){
        int seed = rand();
        init(seed,Z_in);
        NStep = amoebasat(Z_in,x,&sat);
        NStep_avg+=NStep;
        printf("\nNStep = %d ",NStep);
        for(int k=1;k<=N_VARIABLE;k++) cout << x[k];

        if(satisfiable(f1,f1_sign,x)==1) printf("PASSED ");
        else printf("UNSATISFIED ");
    }
   printf("\nAverage #steps = %d ", NStep_avg/i);
   return 0;
}
