#include "ap_int.h"

#define N_VARIABLE 225
#define N_CLAUSE 960
#define N_LITERAL 3
//#define size_contra_max 10*N_CLAUSE*N_CLAUSE/4/N_VARIABLE
//#define size_contra 21
//#define count_var abs(log(N_VARIABLE)/log(2))+1
//#define count_clause abs(log(N_CLAUSEx3)/log(2))+1
const int MAX_N_STEP = 1000000;
const int EPSILON[10] = {10485, 10158, 9830, 9502, 9175, 8847, 8519, 8192, 7864, 7536}; //16bit
const int subtractor = 22283; //1-epsilon

typedef ap_fixed<2,2> largeX_t;
typedef ap_ufixed<1,1> one_bit_t;
typedef ap_ufixed<6,6> f_t;
typedef ap_fixed<16,16> z_t;

int amoebasat(
		z_t Z[N_VARIABLE+1][2],
		char eps_id,
		one_bit_t x[N_VARIABLE+1],
		one_bit_t *sat);
void update_L(one_bit_t L[N_VARIABLE+1][2], largeX_t LargeX[N_VARIABLE+1][2]);
