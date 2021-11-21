#include "amoebasat.h"
#include "local_rules.h"
#include "local_rules_sign.h"
void update_L(one_bit_t L[N_VARIABLE+1][2], largeX_t LargeX[N_VARIABLE+1][2]){
	//Check all rules of unit[1][0]
	one_bit_t light_on10 = LargeX[1][1]>0;
	if(!light_on10){
		for(int i=0;i<14;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit1_0[i][0] ][ unit1_0_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit1_0[i][1] ][ unit1_0_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit1_0[i][2] ][ unit1_0_sign[i][2] ] >0);
			light_on10 = X_contra01 & X_contra23 & X_contra45;
			if(unit1_0[i][4]==0){
				L[1][1] = L[1][1] & (!(X_contra01 & X_contra23));}
		}
	}
	L[1][0] = light_on10;
	//Check all rules of unit[1][1]
	one_bit_t light_on11 = LargeX[1][0]>0;
	if(!light_on11){
		for(int i=0;i<8;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit1_1[i][0] ][ unit1_1_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit1_1[i][1] ][ unit1_1_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit1_1[i][2] ][ unit1_1_sign[i][2] ] >0);
			light_on11 = X_contra01 & X_contra23 & X_contra45;
			if(unit1_1[i][4]==0){
				L[1][0] = L[1][0] & (!(X_contra01 & X_contra23));}
		}
	}
	L[1][1] = light_on11;
	//Check all rules of unit[2][0]
	one_bit_t light_on20 = LargeX[2][1]>0;
	if(!light_on20){
		for(int i=0;i<12;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit2_0[i][0] ][ unit2_0_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit2_0[i][1] ][ unit2_0_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit2_0[i][2] ][ unit2_0_sign[i][2] ] >0);
			light_on20 = X_contra01 & X_contra23 & X_contra45;
			if(unit2_0[i][4]==0){
				L[2][1] = L[2][1] & (!(X_contra01 & X_contra23));}
		}
	}
	L[2][0] = light_on20;
	//Check all rules of unit[2][1]
	one_bit_t light_on21 = LargeX[2][0]>0;
	if(!light_on21){
		for(int i=0;i<8;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit2_1[i][0] ][ unit2_1_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit2_1[i][1] ][ unit2_1_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit2_1[i][2] ][ unit2_1_sign[i][2] ] >0);
			light_on21 = X_contra01 & X_contra23 & X_contra45;
			if(unit2_1[i][4]==0){
				L[2][0] = L[2][0] & (!(X_contra01 & X_contra23));}
		}
	}
	L[2][1] = light_on21;
	//Check all rules of unit[3][0]
	one_bit_t light_on30 = LargeX[3][1]>0;
	if(!light_on30){
		for(int i=0;i<14;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit3_0[i][0] ][ unit3_0_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit3_0[i][1] ][ unit3_0_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit3_0[i][2] ][ unit3_0_sign[i][2] ] >0);
			light_on30 = X_contra01 & X_contra23 & X_contra45;
			if(unit3_0[i][4]==0){
				L[3][1] = L[3][1] & (!(X_contra01 & X_contra23));}
		}
	}
	L[3][0] = light_on30;
	//Check all rules of unit[3][1]
	one_bit_t light_on31 = LargeX[3][0]>0;
	if(!light_on31){
		for(int i=0;i<8;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit3_1[i][0] ][ unit3_1_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit3_1[i][1] ][ unit3_1_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit3_1[i][2] ][ unit3_1_sign[i][2] ] >0);
			light_on31 = X_contra01 & X_contra23 & X_contra45;
			if(unit3_1[i][4]==0){
				L[3][0] = L[3][0] & (!(X_contra01 & X_contra23));}
		}
	}
	L[3][1] = light_on31;
	//Check all rules of unit[4][0]
	one_bit_t light_on40 = LargeX[4][1]>0;
	if(!light_on40){
		for(int i=0;i<11;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit4_0[i][0] ][ unit4_0_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit4_0[i][1] ][ unit4_0_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit4_0[i][2] ][ unit4_0_sign[i][2] ] >0);
			light_on40 = X_contra01 & X_contra23 & X_contra45;
			if(unit4_0[i][4]==0){
				L[4][1] = L[4][1] & (!(X_contra01 & X_contra23));}
		}
	}
	L[4][0] = light_on40;
	//Check all rules of unit[4][1]
	one_bit_t light_on41 = LargeX[4][0]>0;
	if(!light_on41){
		for(int i=0;i<10;i++){
//#pragma HLS PIPELINE
			one_bit_t X_contra01;
			one_bit_t X_contra23;
			one_bit_t X_contra45;
			X_contra01 = ( LargeX[ unit4_1[i][0] ][ unit4_1_sign[i][0] ] >0);
			X_contra23 = ( LargeX[ unit4_1[i][1] ][ unit4_1_sign[i][1] ] >0);
			X_contra45 = ( LargeX[ unit4_1[i][2] ][ unit4_1_sign[i][2] ] >0);
			light_on41 = X_contra01 & X_contra23 & X_contra45;
			if(unit4_1[i][4]==0){
				L[4][0] = L[4][0] & (!(X_contra01 & X_contra23));}
		}
	}
	L[4][1] = light_on41;
}