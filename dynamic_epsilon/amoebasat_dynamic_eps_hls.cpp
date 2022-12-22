#include "amoebasat.h"
#include "f.h"
#include "f_sign.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

void update_L(one_bit_t L[N_VARIABLE+1][2], largeX_t LargeX[N_VARIABLE+1][2]);
one_bit_t sign_x(z_t x){
    if(x>=0) return 1;
    else return 0;
}

void update_Y(one_bit_t Y[N_VARIABLE+1][2], one_bit_t L[N_VARIABLE+1][2], z_t Z[N_VARIABLE+1][2], char eps_id){
	Y_OUTER:
	for(int i=1;i<=N_VARIABLE;i++){
		Y_INNER:
		for(int j=0;j<=1;j++){
			z_t sub = 32768 - EPSILON[ eps_id ]- Z[i][j];
			Y[i][j] = (!L[i][j]) & (sub>0);
		}
	}
}

void update_Z(z_t Z[N_VARIABLE+1][2]){
	Z_OUTER:
	for(int i=1;i<=N_VARIABLE;i++){
		Z_INNER:
		for(int j=0;j<=1;j++){
			one_bit_t bitx = (Z[i][j] & (1<<15)) >> 15;
			bitx ^= (Z[i][j] & (1<<10)) >> 10;
			bitx ^= (Z[i][j] & (1<<5)) >> 5;
						if(Z[i][j]<0x4000){   //if <0.5
							Z[i][j] = Z[i][j]<<1;
							Z[i][j] += bitx;
						}else{
							Z[i][j] = ((0xffff ^ Z[i][j])+1)<<1;
							Z[i][j] += bitx;
						}
			//cout<<"Z["<<i<<"]["<<j<<"]="<<Z[i][j]<<"\n";
		}
	}
}
void update_LargeX(one_bit_t Y[N_VARIABLE+1][2], largeX_t LargeX[N_VARIABLE+1][2]){
    int i, j;
    LargeX_OUTER:
    for(i=1; i<=N_VARIABLE; i++){
        LargeX_INNER:
    	for(j=0; j<=1; j++){
            if(Y[i][j]==1 && LargeX[i][j]<1){
                LargeX[i][j]++;
            }else if(Y[i][j]==0 && LargeX[i][j]>-1){
                LargeX[i][j]--;
            }
            //cout<<"X["<<i<<"]"<<j<<LargeX[i][j]<<"\n";
        }
    }
}
void update_x(one_bit_t x_tmp[N_VARIABLE+1], largeX_t LargeX[N_VARIABLE+1][2]){
	int i;
	x_OUTER:
	for(i=1; i<=N_VARIABLE; i++){
		x_INNER:
		if(LargeX[i][0]==1 && LargeX[i][1]<=0){
			x_tmp[i] = 0;
		}else if(LargeX[i][1]==1 && LargeX[i][0]<=0){
			x_tmp[i] = 1;
		}

		//one_bit_t X0 = LargeX[i][0]>0;
		//one_bit_t X1 = LargeX[i][1]>0;
		//x_tmp[i] = X0 xor X1;
		//cout<<"x"<<i<<"="<<x_tmp[i]<<" ";
	}
}
one_bit_t check_f(f_t f[N_VARIABLE][3], one_bit_t f_sign[N_VARIABLE][3], one_bit_t x[N_VARIABLE+1]){
	int clause_id, k;
	one_bit_t c0,c1,c2;
	one_bit_t f_val=1;
	Loop_update_f:
	for(clause_id=0; clause_id<402; clause_id++){
		c0 = x[ f[clause_id][0] ] ^ f_sign[clause_id][0];
		if(f[clause_id][1]>0)c1 = x[ f[clause_id][1] ] ^ f_sign[clause_id][1];
		else c1=0;
		c2 = x[ f[clause_id][2] ] ^ f_sign[clause_id][2];
		//if(clause_id<2) cout << "clause" << clause_id+1 << " "<<inter[clause_id][0]<<" "<<inter[clause_id][1]<<" "<<c0<<" "<<c1<<" "<<c2<<"\n";
		f_val=f_val&(c0|c1|c2);
		if(!f_val) {
			//cout << "unsatisfied clause " << clause_id+1 << " "<<inter[clause_id][0]<<" "<<inter[clause_id][1]<<" "<<c0<<" "<<c1<<" "<<c2<<"\n";
			break;
		}
	}
	//if(f_val==1) printf("ok");
	return f_val;
}

int amoebasat(
		z_t Z[N_VARIABLE+1][2],
		char eps_id,
		one_bit_t x[N_VARIABLE+1],
		one_bit_t *sat)
{
#pragma HLS ARRAY_PARTITION variable=x dim=0
    z_t Z_tmp[N_VARIABLE+1][2];
#pragma HLS ARRAY_PARTITION variable=Z_tmp dim=0
	int NStep = 0;
	int i,k;
	one_bit_t x_tmp[N_VARIABLE+1];
#pragma HLS ARRAY_PARTITION variable=x_tmp dim=0
	one_bit_t Y[N_VARIABLE+1][2];
#pragma HLS ARRAY_PARTITION variable=Y dim=0
	one_bit_t L[N_VARIABLE+1][2];
#pragma HLS ARRAY_PARTITION variable=L dim=0
	largeX_t LargeX[N_VARIABLE+1][2];
#pragma HLS ARRAY_PARTITION variable=LargeX dim=0
	one_bit_t sat_temp=0;

	//initialize
	LargeX[0][0]=1;
	LOOP_INIT:
	for(i=1;i<=N_VARIABLE;i++){
#pragma HLS PIPELINE
		Z_tmp[i][0] = Z[i][0];
		Z_tmp[i][1] = Z[i][1];
		Y[i][0] = 0; Y[i][1]=0;
		LargeX[i][0] = 0; LargeX[i][1] = 0;
		L[i][0] = 0; L[i][1] = 0;
	}

#pragma HLS DEPENDENCE variable=L array inter WAR false
#pragma HLS DEPENDENCE variable=LargeX array inter WAR false
#pragma HLS DEPENDENCE variable=Z array inter WAR false
	bool done = false;
	LOOP_UPDATE_ALL:
	for(NStep=0;!done;NStep++){
#pragma HLS PIPELINE
		update_Y(Y, L, Z_tmp, eps_id);
		update_Z(Z_tmp);
		update_L(L,LargeX);
		update_LargeX(Y,LargeX);
		update_x(x_tmp,LargeX);
		sat_temp = check_f(f,f_sign,x_tmp);//cout<<"sat_temp="<<sat_temp<<"\n";
		done = (sat_temp==1) | (NStep>=MAX_N_STEP);

		//print
		/*for(i=6;i<=10;i++){
			for(k=0;k<2;k++){
				cout <<"Z["<<i<<"]["<<k<<"]="<<Z[i][k]<<"\t";
				cout <<"Y["<<i<<"]["<<k<<"]="<<Y[i][k]<<"\t";
				cout <<"LargeX["<<i<<"]["<<k<<"]="<<LargeX[i][k]<<"\t";
				cout <<"L["<<i<<"]["<<k<<"]="<<L[i][k]<<"\n";

			}
			cout << "x["<<i<<"]="<<x_tmp[i]<<"\n";
		}*/
	}

	//write output
	LOOP_WRITE:
	for(k=1;k<=N_VARIABLE;k++){
#pragma HLS PIPELINE
		x[k] = x_tmp[k];
	}
	*sat = sat_temp;
	return NStep;
}
