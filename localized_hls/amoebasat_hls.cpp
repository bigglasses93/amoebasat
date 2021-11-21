#include "amoebasat.h"
#include "f.h"
#include "f_sign.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

void update_Y(one_bit_t Y[N_VARIABLE+1][2], one_bit_t L[N_VARIABLE+1][2], z_t Z[N_VARIABLE+1][2]){
	Y_OUTER:
	for(int i=1;i<=N_VARIABLE;i++){
		Y_INNER:
		for(int j=0;j<=1;j++){
			z_t sub = subtractor - Z[i][j];
			Y[i][j] = (!L[i][j]) & (sub>0);
		}
	}
}

void update_Z(z_t Z[N_VARIABLE+1][2]){
	Z_OUTER:
	for(int i=1;i<=N_VARIABLE;i++){
		Z_INNER:
		for(int j=0;j<=1;j++){
			one_bit_t bit = (Z[i][j] & (1<<15)) >> 15;
						bit ^= (Z[i][j] & (1<<10)) >> 10;
						bit ^= (Z[i][j] & (1<<5)) >> 5;
						if(Z[i][j]<0x4000){   //if <0.5
							Z[i][j] = Z[i][j]<<1;
							Z[i][j] += bit;
						}else{
							Z[i][j] = (0x8000 - Z[i][j])<<1;
							Z[i][j] += bit;
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
		one_bit_t X0 = LargeX[i][0]>0;
		one_bit_t X1 = LargeX[i][1]>0;
		x_tmp[i] = X0 xor X1;
		//cout<<"x"<<i<<"="<<x_tmp[i]<<" ";
	}
}
one_bit_t check_f(f_t inter[N_CLAUSE][N_LITERAL], one_bit_t inter_sign[N_CLAUSE][N_LITERAL], one_bit_t x[N_VARIABLE+1]){
	int clause_id, k;
	one_bit_t c_val;
	one_bit_t f_val=1;
	Loop_update_f:
	for(clause_id=0; clause_id<N_CLAUSE; clause_id++){
		c_val = x[ inter[clause_id][0] ] ^ inter_sign[clause_id][0];
		for(k=1;k<3;k++){
			c_val = c_val | (x[ inter[clause_id][k] ] ^ inter_sign[clause_id][k]);
		}
		f_val=f_val&c_val;
		if(!f_val) {
			break;
		}
	}
	//if(f_val==1) printf("ok");
	return f_val;
}


int amoebasat(
		z_t Z[N_VARIABLE+1][2],
		one_bit_t x[N_VARIABLE+1],
		one_bit_t sat)
{
	int NStep = 0;
	one_bit_t x_tmp[N_VARIABLE+1];
#pragma HLS ARRAY_PARTITION variable=x_tmp complete dim=0
	one_bit_t Y[N_VARIABLE+1][2];
#pragma HLS DEPENDENCE variable=Y array inter WAR false
#pragma HLS DEPENDENCE variable=x_tmp array inter WAR false
	one_bit_t L[N_VARIABLE+1][2];
#pragma HLS DEPENDENCE variable=L array inter WAR false
	largeX_t LargeX[N_VARIABLE+1][2];
	z_t Z_tmp[N_VARIABLE+1][2];
	one_bit_t sat_temp=0;
	
	//initialize
	LargeX[0][0]=1;
	LOOP_INIT:
	for(int i=1;i<=N_VARIABLE;i++){
#pragma HLS PIPELINE
		Z_tmp[i][0] = Z[i][0];
		Z_tmp[i][1] = Z[i][1];
	}

	//for(NStep=0;NStep<MAX_N_STEP;NStep++){
	bool done = false;
	LOOP_UPDATE_ALL:
	for(NStep=0;!done;NStep++){
	//for(NStep=0;(NStep<MAX_N_STEP)&&(sat_temp==0);NStep++){
		update_Z(Z_tmp);
		update_LargeX(Y,LargeX);
		update_Y(Y, L, Z_tmp);
		update_L(L,LargeX);
		update_x(x_tmp,LargeX);
		sat_temp = check_f(f,f_sign,x_tmp);//cout<<"sat_temp="<<sat_temp<<"\n";
{
		#pragma HLS LATENCY min=1 max=1
}
		done = (sat_temp==1) | (NStep>=MAX_N_STEP);
	}
	
	//write output
	LOOP_WRITE:
	for(int k=1;k<=N_VARIABLE;k++){
#pragma HLS PIPELINE
		x[k] = x_tmp[k];
	}
	sat = sat_temp;
	return NStep;
}


