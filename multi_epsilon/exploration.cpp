#include<stdio.h>
//#include<stdlib.h>
#include<string.h>
#include<time.h>
#define N_VARIABLE 90
#define N_CLAUSE 300
#define N_LITERAL 3
const int MAX_N_STEP = 1000000;
//epsilon range from 0.32 -> 0.12
const int eps1[22] = {901943132,880468296,858993459,837518623,816043786,794568950,773094113,751619277,730144440,708669604,687194767,665719931,644245094,622770258,601295421,579820585,558345748,536870912,515396076,493921239,472446403,450971566};
const int eps[20] = {665719931,644245094,622770258,601295421,579820585,558345748,536870912,515396076,493921239,472446403,450971566,429496730,408021893,386547057,365072220,343597384,322122547,300647711,279172874,257698038};

int EPSILON1;
int EPSILON2;
#define MAX_CONTRA 80000
#define THRES 13

int conn[N_VARIABLE+1][2]; //connections# of all variables
int LargeX[N_VARIABLE+1][2];
int Y[N_VARIABLE+1][2];
int L[N_VARIABLE+1][2];
int Z[N_VARIABLE+1][2];
int f[N_CLAUSE][N_LITERAL];
char x[N_VARIABLE+1];
char x_fixed[N_VARIABLE+1];
int inter[3*N_CLAUSE][6];
int size_inter;
int size_contra;
int size_contra_new;
int contra_new[MAX_CONTRA][8];

unsigned state[1];

void init();
void update_Y();
void update_Z();
int update_f();
void update_LargeX();
void update_x();
int sign_x(int x);
int loadformula(char *filename);
void generate_inter();
int survey_size_contra();
void generate_contra(int size_contra, int contra[size_contra][8], int contra_new[MAX_CONTRA][8]);
void update_L_intra();
void update_L_inter(int inter[3*N_CLAUSE][6]);
void update_L_contra(int size_contra, int contra[size_contra][8]);
int amoebasat(char s[N_VARIABLE+100]);
void create_local_rules(int inter[3*N_CLAUSE][6], int contra_new[MAX_CONTRA][8]);

FILE *fp1; //z
FILE *fp2; //inter
FILE *fp3; //contra
FILE *fp4; //local rules

int main() {
    char filename[128]="filename.cnf";
    char logfile[128]= "log.txt";

    char s[N_VARIABLE+100]; //output solution
    fp1 = fopen(logfile,"w+");
    int i, k, l;

    if (!loadformula(filename)) {
        return 1;
    }
    srand(time(NULL));
    generate_inter();
    size_contra = survey_size_contra();
    int contra[size_contra][8];
    generate_contra(size_contra, contra, contra_new);
    create_local_rules(inter,contra_new);
//for(l=10;l>=0;l--){
    EPSILON1 = eps1[7];
    printf("epsilon0 = %d", EPSILON1);
    for(k=0;k<10;k++){
        EPSILON2 = eps[k];
        printf("\tepsilon1 = %d", eps[k]);
        int NStep_avg = 0;
        int NStep = 0;
        for(i=0;i<10;i++){
            state[0] = rand();
            //printf("\nRun %d: seed = %d",i+1,state[0]);
            fprintf(fp1,"\nRun #%d: seed = %d ",i+1,state[0]);

            init();
            NStep=amoebasat(s);
            fprintf(fp1,s);
            NStep_avg+=NStep;
        }
        printf("\nAverage #iterations = %d\n", NStep_avg/i);
        fprintf(fp1,"\nAverage #iterations = %d", NStep_avg/i);
        //for(i=1;i<=N_VARIABLE;i++) printf("%d", x[i]);   
    }
        fclose(fp1);
        return 0;
    }
//}
int loadformula(char *filename) {
    FILE *fp;
    char str[255];
    char *token;
    int j; // i = order of token; j = 0,1,2 -> 3 literals per clause
    int clause_id = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File not found!");
        return 0;
    }
    else {
        printf("%s ", filename);
        while (!feof(fp)) {
            fgets(str, 255, fp);
            // get N_LITERAL, N_CLAUSE
            if (strstr(str, "p cnf")) {
                continue;
            }
            //skip reading first lines begin with c
            else if (strstr(str, "c"))
                continue;
            // get Clause parameters
            else {
                token = strtok(str, " ,\t\n");
                if (strstr(str, "%"))
                    break;
                f[clause_id][0] = atoi(token);
                for (j = 1; j < N_LITERAL; j++) {
                    token = strtok(NULL, " ,\t\n");
                    f[clause_id][j] = atoi(token);      //f[literal][clause_id]
                    if(j==1 && f[clause_id][j]==0){
                        int xi = abs(f[clause_id][0]);
                        int xi_sign = sign_x(f[clause_id][0]);
                        x[xi] = 1-xi_sign;
                        x_fixed[xi]=1;
                        //printf("x[%d] = %d; x_fixed[%d] = %d; satisfiable[%d][0]=1; satisfiable[%d][1]=1;\n", xi, x[xi], xi, x[xi], xi, xi);
                        printf("x_tmp[%d] = %d; x_fixed[%d] = 1;\n", xi, x[xi], xi);
                    }
                }
                //printf("\nClause%d:\t%d\t%d\t%d",clause_id,f[clause_id][0],f[clause_id][1],f[clause_id][2]);
                clause_id++;
            }
        }
        fclose(fp);
    }
    return 1;
}
int amoebasat(char s[N_VARIABLE+100]){
    char tmp[N_VARIABLE+100];
    int NStep;
    LargeX[0][0]=1; x[0]=0;
    for(NStep=1;NStep<MAX_N_STEP;NStep++){
        update_L_intra();
        update_L_inter(inter);
        update_L_contra(size_contra, contra_new);
        int i,j;
        update_Z();//printf("\n");
        //update_Y();
        update_LargeX();
        update_x();

        if(update_f()){
            sprintf(s,"\nFound: %d iterations ", NStep);
            //for(i=0;i<N_VARIABLE;i++) printf("%d",x[i]);
            for(i=1;i<=N_VARIABLE;i++) {
                sprintf(tmp,"%d",x[i]);
                strcat(s,tmp);
            }
            //printf("%s",s);
            return NStep;
        }
    }
    //printf("\nNot found.");

    return NStep;
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
void init(){
    int i, j;
    for(i=1;i<=N_VARIABLE;i++){
        if(x_fixed[i]==0) x[i] = 0;
        for(j=0;j<2;j++){
            LargeX[i][j]=0;
            L[i][j]=0;
            while(Z[i][j]==0||Z[i][j]==0x40000000UL){
                Z[i][j] = xorshift32(state)>>1;
            }
            //printf("Z[%d][%d]=%d\t",i,j,Z[i][j]);
        }
    }
}
void update_Z(){
    int i, j;
    for(i=1; i<=N_VARIABLE; i++){
        for(j=0; j<2; j++){
            //input bit = x30 ^ x20 ^ x10
            int bit30 = (Z[i][j] & ( 1 << 30 )) >> 30;
            int bitx = (Z[i][j] & ( 1 << 20 )) >> 20;
            bit30 = bit30 ^ bitx;
            bitx = (Z[i][j] & ( 1 << 10 )) >> 10;
            bit30 = bit30 ^ bitx;
            if(Z[i][j]<0x40000000UL){   //if <0.5
                Z[i][j] = Z[i][j]<<1;
                Z[i][j] += bit30;

            }else{
                Z[i][j] = ((0xffffffffUL ^ Z[i][j])+1)<<1;
                Z[i][j] += bit30;
            }
            //printf("Z[%d][%d]=%d\t",i,j,Z[i][j]);
        }
    }
}
void update_x(){
    int i;
    for(i=1; i<=N_VARIABLE; i++){
        if(x_fixed[i]==0 && LargeX[i][0]==1 && LargeX[i][1]<=0){
            x[i] = 0;
        }else if(x_fixed[i]==0 && LargeX[i][1]==1 && LargeX[i][0]<=0){
            x[i] = 1;
        }
        //printf("x[%d]=%d \t", i, x[i]);
    }
}
void update_Y(){
    int i, j;
    for(i=1;i<=N_VARIABLE;i++){
        for(j=0; j<2; j++){
			int sub = 2147483647 - EPSILON1 - Z[i][j] +1;
			Y[i][j] = (!L[i][j]) & (sub>0);
            //printf("Y[%d][%d]=%d\t ",i,j,Y[i][j]);
        }
    }
}
void update_LargeX(){
    int i,j;
    //New version of LargeX (merged with Y)
    for(i=1;i<=N_VARIABLE;i++){
        for(j=0;j<=1;j++){
            if(L[i][j]==1 && LargeX[i][j]>-1) LargeX[i][j]--;
            else if(L[i][j]==0){
                int sub;
                if(conn[i][j]<THRES) sub = 2147483647 - EPSILON1 - Z[i][j] +1;
                else sub = 2147483647 - EPSILON2 - Z[i][j] +1;
                //if Z > 1- epsilon, X expands; otherwise shrink
                if(sub>0 && LargeX[i][j]<1){
                    LargeX[i][j]++;
                }else if(sub<0 && LargeX[i][j]>-1){
                    LargeX[i][j]--;
                }
            }else if(L[i][j]==2) LargeX[i][j]==-1;
        }
    }

    /*for(i=0;i<N_VARIABLE;i++){
        for(j=0;j<=1;j++){
            if(Y[i][j]==1 && LargeX[i][j]<1){
                LargeX[i][j]++;
            }else if(Y[i][j]==0 && LargeX[i][j]>-1){
                LargeX[i][j]--;
            }
        }
    }*/
}
int sign_x(int x){
    if(x>=0) return 0;
    else return 1;
}
int update_f(){
    int clause_id;
    int f_val = 1;
    for(clause_id=0; clause_id<N_CLAUSE; clause_id++){
        int id1, id2, id3;
        int c1, c2, c3;
        id1 = f[clause_id][0];
        id2 = f[clause_id][1];
        id3 = f[clause_id][2];
        if(id2==0) continue;
        c1 = (id1<0) ^ x[abs(id1)];
        c2 = (id2<0) ^ x[abs(id2)];
        if(id3==0){
            c3 = 0;
        }else{
            c3 = (id3<0) ^ x[abs(id3)];
        }
        f_val = f_val && (c1 || c2 || c3);
        if (f_val==0) {
            //printf("unsat clause id = %d: %d %d %d \n",clause_id, id1, id2, id3);
            return f_val;
        }
    }
    return f_val;
}
void generate_inter(){
    int i;
    fp2 = fopen("inter.h", "w+");
    fp3 = fopen("inter_sign.h", "w+");
    fp4 = fopen("f_tmp.txt", "w+");
    //fprintf(fp2, "int interP2[%d][2] = {\n", 3*N_CLAUSE);

    fprintf(fp2,"f_t inter[N_CLAUSE][3]={\n");
    fprintf(fp3,"one_bit_t inter_sign[N_CLAUSE][3]={\n");
    size_inter=0;
    for(i=0; i<N_CLAUSE; i++){
        //-------- generate inter set size 3x
        int interQ, interQ_abs, interP1, interP1_abs, interP2, interP2_abs;
        if(f[i][1]==0) {
			//printf("\nClause%d:\t%d\t%d\t%d",i,f[i][0],f[i][1],f[i][2]);
			continue;
		}

        interQ = abs(f[i][0]); interQ_abs = sign_x(f[i][0]);
        interP1= abs(f[i][1]); interP1_abs = sign_x(f[i][1]);
        interP2 = abs(f[i][2]); interP2_abs = sign_x(f[i][2]);

        inter[3*size_inter][0] = interP1; inter[3*size_inter][1] = interP1_abs;
        inter[3*size_inter][2] = interP2; inter[3*size_inter][3] = interP2_abs;
        inter[3*size_inter][4] = interQ; inter[3*size_inter][5] = interQ_abs;

        inter[3*size_inter+1][0] = interQ; inter[3*size_inter+1][1] = interQ_abs;
        inter[3*size_inter+1][2] = interP2; inter[3*size_inter+1][3] = interP2_abs;
        inter[3*size_inter+1][4] = interP1; inter[3*size_inter+1][5] = interP1_abs;

        inter[3*size_inter+2][0] = interQ; inter[3*size_inter+2][1] = interQ_abs;
        inter[3*size_inter+2][2] = interP1; inter[3*size_inter+2][3] = interP1_abs;
        inter[3*size_inter+2][4] = interP2; inter[3*size_inter+2][5] = interP2_abs;

        size_inter++;
        //--------- generate inter set size 1x
        //fprintf(fp2, "%d %d %d %d %d %d\n", inter[3*i][0], inter[3*i][1], inter[3*i][2], inter[3*i][3], inter[3*i][4], inter[3*i][5]);
        if(i==(N_CLAUSE-1)) fprintf(fp2, "%d, %d, %d};", interP1, interP2, interQ);
        else fprintf(fp2, "%d, %d, %d,\n", interP1, interP2, interQ);

        if(i==(N_CLAUSE-1)) fprintf(fp3, "%d, %d, %d};", interP1_abs, interP2_abs, interQ_abs);
        else fprintf(fp3, "%d, %d, %d,\n", interP1_abs, interP2_abs, interQ_abs);

        fprintf(fp4, "%d %d %d %d %d %d\n", interQ, interQ_abs, interP1, interP1_abs, interP2, interP2_abs);
    }
    printf("size inter = %d\n", size_inter);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
}
int survey_size_contra(){
    //Survey size of contra
    int i,j;
    int size_contra=0;
    for(i=0; i<3*size_inter; i=i+3){
        for(j=i+3; j<3*size_inter; j++){
            if(inter[i][4]==inter[j][4] && inter[i][5]!=inter[j][5]) size_contra++;
            if(inter[i+1][4]==inter[j][4] && inter[i+1][5]!=inter[j][5]) size_contra++;
            if(inter[i+2][4]==inter[j][4] && inter[i+2][5 ]!=inter[j][5]) size_contra++;
        }
    }
    printf("Size contra survey = %d\n", size_contra);
    return size_contra;
}
void generate_contra(int size_contra, int contra[size_contra][8], int contra_new[MAX_CONTRA][8]){
    int i,j,k;
    int size_contra0, size_contra1, counter;
    counter = 0;

    int contra0[N_CLAUSE][4];
    int contra1[N_CLAUSE][4];
    for(i=1; i<=N_VARIABLE; i++){
        size_contra0 = 0;
        size_contra1 = 0;
        for(j=0; j<3*size_inter; j++){
            if(inter[j][4]==i){
                if(inter[j][5]==0){
                    contra0[size_contra0][0] = inter[j][0];
                    contra0[size_contra0][1] = inter[j][1];
                    contra0[size_contra0][2] = inter[j][2];
                    contra0[size_contra0][3] = inter[j][3];
                    //printf("\ncontra0[%d]: %d%d , %d%d", size_contra0, contra0[size_contra0][0], contra0[size_contra0][1], contra0[size_contra0][2], contra0[size_contra0][3]);
                    size_contra0++;
                }else{
                    contra1[size_contra1][0] = inter[j][0];
                    contra1[size_contra1][1] = inter[j][1];
                    contra1[size_contra1][2] = inter[j][2];
                    contra1[size_contra1][3] = inter[j][3];
                    //printf("\ncontra1[%d]: %d%d , %d%d", size_contra1, contra1[size_contra1][0], contra1[size_contra1][1], contra1[size_contra1][2], contra1[size_contra1][3]);
                    size_contra1++;
                }
            }
        }
        for(j=0; j<size_contra0; j++){
            for(k=0; k<size_contra1; k++){
                contra[counter][0] = contra0[j][0];
                contra[counter][1] = contra0[j][1];
                contra[counter][2] = contra0[j][2];
                contra[counter][3] = contra0[j][3];

                contra[counter][4] = contra1[k][0];
                contra[counter][5] = contra1[k][1];
                contra[counter][6] = contra1[k][2];
                contra[counter][7] = contra1[k][3];

                counter++;
            }
        }
    }
    //clear redundant lines from contra: if there's a line containing 2 counterparts of a unit, for ex 8 0 8 1, the rule is never contradicted, so no need to check
    int counter_new = 0;
    for(k=0; k<size_contra; k++){
        if(contra[k][0]==contra[k][4] && contra[k][1]==(1-contra[k][5]) ) {
            //printf("\nc1=c3 %d %d , %d %d , %d %d , %d %d", contra[k][0], contra[k][1], contra[k][2], contra[k][3], contra[k][4], contra[k][5], contra[k][6], contra[k][7]);
            continue;
        }
        else if(contra[k][0]==contra[k][6] && contra[k][1]==(1-contra[k][7]) ) {
            //printf("\nc1=c4 %d %d , %d %d , %d %d , %d %d", contra[k][0], contra[k][1], contra[k][2], contra[k][3], contra[k][4], contra[k][5], contra[k][6], contra[k][7]);
            continue;
        }
        else if(contra[k][2]==contra[k][4] && contra[k][3]==(1-contra[k][5]) ) {
            //printf("\nc2=c3 %d %d , %d %d , %d %d , %d %d", contra[k][0], contra[k][1], contra[k][2], contra[k][3], contra[k][4], contra[k][5], contra[k][6], contra[k][7]);
            continue;
        }
        else if(contra[k][2]==contra[k][6] && contra[k][3]==(1-contra[k][7]) ) {
            //printf("\nc2=c4 %d %d , %d %d , %d %d , %d %d", contra[k][0], contra[k][1], contra[k][2], contra[k][3], contra[k][4], contra[k][5], contra[k][6], contra[k][7]);
            continue;
        }
        contra_new[counter_new][0] = contra[k][0];
        contra_new[counter_new][1] = contra[k][1];
        contra_new[counter_new][2] = contra[k][2];
        contra_new[counter_new][3] = contra[k][3];
        contra_new[counter_new][4] = contra[k][4];
        contra_new[counter_new][5] = contra[k][5];
        contra_new[counter_new][6] = contra[k][6];
        contra_new[counter_new][7] = contra[k][7];
        counter_new++;
    }
    size_contra=counter_new;
    printf("Size contra = %d\n", size_contra);
    //write to file

    fp2 = fopen("contra.h", "w+");
    fp3 = fopen("contra_sign.h", "w+");
    fprintf(fp2, "f_t contra[%d][4] = {\n", size_contra);
    fprintf(fp3, "one_bit_t contra_sign[%d][4] = {\n", size_contra);
    for(i=0;i<counter_new;i++){
        //fprintf(fp3, "%d %d %d %d %d %d %d %d\n", contra_new[i][0], contra_new[i][1], contra_new[i][2], contra_new[i][3], contra_new[i][4], contra_new[i][5], contra_new[i][6], contra_new[i][7]);
        if(i!=(counter_new-1)){
            fprintf(fp2, "%d, %d, %d, %d,\n", contra_new[i][0], contra_new[i][2], contra_new[i][4], contra_new[i][6]);
            fprintf(fp3, "%d, %d, %d, %d,\n", contra_new[i][1], contra_new[i][3], contra_new[i][5], contra_new[i][7]);
        }
        else {
            fprintf(fp2, "%d, %d, %d, %d};", contra_new[i][0], contra_new[i][2], contra_new[i][4], contra_new[i][6]);
            fprintf(fp3, "%d, %d, %d, %d};", contra_new[i][1], contra_new[i][3], contra_new[i][5], contra_new[i][7]);
        }
    }
    fclose(fp2);
    fclose(fp3);
    size_contra_new=size_contra;
}
void update_L_intra(){
    int i,j;
    for(i=1;i<=N_VARIABLE;i++){
    	L[i][0] = (LargeX[i][1]>0);
    	L[i][1] = (LargeX[i][0]>0);
    }
}
void update_L_inter(int inter[3*N_CLAUSE][6]){
    int i;
    for(i=0;i<3*size_inter;i++){
        //printf("\nClause%d:\t%d\t%d\t%d",i,f[i][0],f[i][1],f[i][2]);
        int id1 = inter[i][0];
        int id2 = inter[i][2];
        int inter1;
        if(id1>0 && id2>0){
            inter1 = (LargeX[ inter[i][2] ][ inter[i][3] ] >0) & (LargeX[ inter[i][0] ][ inter[i][1] ] >0);
        }else if(id1==0){
            inter1 = (LargeX[ inter[i][2] ][ inter[i][3] ] >0);
        }else if(id2==0){
            inter1 = (LargeX[ inter[i][0] ][ inter[i][1] ] >0);
        }else if(id1==0 && id2==0){
            inter1 = 0;
        }
        //INTER
        L[ inter[i][4] ][ inter[i][5] ] = L[ inter[i][4] ][ inter[i][5] ] | inter1;
        //COLLAPSE
        L[ inter[i][4] ][ 1-inter[i][5] ] = L[ inter[i][4] ][ 1-inter[i][5] ] & (!inter1);

    }
}
void update_L_contra(int size_contra, int contra[size_contra][8]){
    int i, j;
    for(i=0;i<size_contra;i++){
        int contra1;
        //int id1 = contra[i][0];
        int id2 = contra[i][2];
        //int id3 = contra[i][4];
        int id4 = contra[i][6];
        if(id2==0 && id4==0){
            contra1 = (LargeX[ contra[i][0] ][ contra[i][1] ]>0) & (LargeX[ contra[i][4] ][ contra[i][5] ]>0);
        }else if(id2==0){
            contra1 = (LargeX[ contra[i][0] ][ contra[i][1] ]>0) & (LargeX[ contra[i][4] ][ contra[i][5] ]>0) & (LargeX[ contra[i][6] ][ contra[i][7] ]>0);
        }else if(id4==0){
            contra1 = (LargeX[ contra[i][0] ][ contra[i][1] ]>0) & (LargeX[ contra[i][2] ][ contra[i][3] ]>0) & (LargeX[ contra[i][4] ][ contra[i][5] ]>0);
        }else{
            contra1 = (LargeX[ contra[i][0] ][ contra[i][1] ]>0) &
                (LargeX[ contra[i][2] ][ contra[i][3] ]>0) &
                (LargeX[ contra[i][4] ][ contra[i][5] ]>0) &
                (LargeX[ contra[i][6] ][ contra[i][7] ]>0);
        }

        //CONTRA - Light on CONTRA units - not good with CONFLICT only
        if(contra1){
            L[ contra[i][0] ][ contra[i][1] ] = 1;
            L[ contra[i][2] ][ contra[i][3] ] = 1;
            L[ contra[i][4] ][ contra[i][5] ] = 1;
            L[ contra[i][6] ][ contra[i][7] ] = 1;
        }
    }
}
//create local rules and local modules
void create_local_rules(int inter[3*N_CLAUSE][6], int contra_new[MAX_CONTRA][8]){
    int i, j; //unit index
    int k, l, m, n;
    int count_rules[N_VARIABLE+1][2];
    char s[1000000]; //temporary text
    char s1[100]; //text for 1 line
    fp4 = fopen("local_rules.txt", "w+");
    //write all rules for unit X[1][0]
    for(i=1;i<=N_VARIABLE;i++){
        if(x_fixed[i]==1) continue;
        for(j=0;j<=1;j++){
            //initialize connections# of unit[i][j]
            int connections[N_VARIABLE+1][2]; //connections# of only unit[i][j], each element represent wire (0 or 1) between unit [i][j] and another unit
            int connection_count=0;
            for (k=0;k<N_VARIABLE;k++){
                connections[k][0]=0;
                connections[k][1]=0;
            }
            count_rules[i][j]=0;
            count_rules[i][j]=0;
            strcpy(s,"");
            strcpy(s,"");
            //fprintf(fp4, "int unit%d_%d[][]={\n", i, j);
            //write rules from inter: each line stores inter rule of 3 elements, if [i][j] is seen in the rule, write down its 2 other elements
            for(k=0;k<N_CLAUSE;k++){
                if(inter[3*k][0]==i && inter[3*k][1]==j){
                    //fprintf(fp4, "%d, %d, %d, %d, 0, 0,\n", inter[3*k][2], inter[3*k][3], inter[3*k][4], inter[3*k][5]);
                    sprintf(s1,"%d, %d, %d, %d, 0, 0,\n", inter[3*k][2], inter[3*k][3], inter[3*k][4], inter[3*k][5]);
                    strcat(s,s1);
                    count_rules[i][j]++;
                    //write to connections
                    connections[ inter[3*k][2] ][ inter[3*k][3] ] = 1;
                    connections[ inter[3*k][4] ][ inter[3*k][5] ] = 1;
                }
                if(inter[3*k][2]==i && inter[3*k][3]==j){
                    sprintf(s1, "%d, %d, %d, %d, 0, 0,\n", inter[3*k][0], inter[3*k][1], inter[3*k][4], inter[3*k][5]);
                    strcat(s,s1);
                    count_rules[i][j]++;
                    //write to connections
                    connections[ inter[3*k][0] ][ inter[3*k][1] ] = 1;
                    connections[ inter[3*k][4] ][ inter[3*k][5] ] = 1;
                }
                if(inter[3*k][4]==i && inter[3*k][5]==j){
                    sprintf(s1, "%d, %d, %d, %d, 0, 0,\n", inter[3*k][0], inter[3*k][1], inter[3*k][2], inter[3*k][3]);
                    strcat(s,s1);
                    count_rules[i][j]++;
                    //write to connections
                    connections[ inter[3*k][0] ][ inter[3*k][1] ] = 1;
                    connections[ inter[3*k][2] ][ inter[3*k][3] ] = 1;
                }
            }
            //write rules from contra: each line stores contra rule of up to 4 elements, if [i][j] is seen in the rule, write down its 3 other elements
            for(k=0;k<size_contra;k++){
                for(l=0;l<8;l=l+2){
                    if(contra_new[k][l]==i && contra_new[k][l+1]==j){
                        //int ok=1;
                        for(m=0;m<8;m=m+2){
                            if(m==l) continue;
                            if(contra_new[k][m]==i && contra_new[k][m+1]==j){
                                sprintf(s1, "0, 0, ");
                                strcat(s,s1);
                                continue;
                            }
                            sprintf(s1, "%d, %d, ", contra_new[k][m], contra_new[k][m+1]);
                            strcat(s,s1);
                            //write to connections
                            connections[ contra_new[k][m] ][ contra_new[k][m+1] ] = 1;
                        }
                        sprintf(s1,"\n");
                        strcat(s,s1);
                        count_rules[i][j]++;
                        break;
                    }
                }
                if(k==size_contra-1) {
                        char* p = s;
                        p[strlen(p)-3]=0;
                        sprintf(s1, "};\n");
                        strcat(s,s1);
                }
            }
            //Count connections#
            for(k=1;k<=N_VARIABLE;k++){
                connection_count+=connections[k][0] + connections[k][1];
                conn[i][j] = connection_count;
            }
            //print out connections# of unit[i][j]
            printf("unit(%d,%d) %d\n",i,j,connection_count);
            //write rules into file
            fprintf(fp4, "int unit%d_%d[%d][6]={\n", i, j,count_rules[i][j]);
            fprintf(fp4,s);
        }
    }
    fclose(fp4);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //version plain
    fp4 = fopen("rules_update.cpp", "w+");
    fprintf(fp4,"#include \n");
    fprintf(fp4,"void update_L(one_bit_t L[N_VARIABLE+1][2], largeX_t LargeX[N_VARIABLE+1][2]){\n");
    fprintf(fp4,"\t//Check INTRA rules of all units\n");
    fprintf(fp4,"\tfor(int i=1;i<=N_VARIABLE;i++){\n");
    fprintf(fp4,"\t\tL[i][0]=LargeX[i][1]>0;L[i][1]=LargeX[i][0]>0;\n\t}\n");
    fprintf(fp4,"\t//Check INTER rules of all units\n");
    for(i=0;i<size_inter;i++){
        strcpy(s,"");
        if(inter[3*i][2]==0){
            /*sprintf(s1,"\tone_bit_t inter%d_0=LargeX[%d][%d]>0;\n",i,inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tone_bit_t inter%d_2=LargeX[%d][%d]>0;\n",i,inter[3*i][0],inter[3*i][1]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | inter%d_0;\n",inter[3*i][0],inter[3*i][1],inter[3*i][0],inter[3*i][1],i); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | inter%d_2;\n",inter[3*i][4],inter[3*i][5],inter[3*i][4],inter[3*i][5],i); strcat(s,s1);
            //COLLAPSE
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!inter%d_0);\n",inter[3*i][0],1-inter[3*i][1],inter[3*i][0],1-inter[3*i][1],i); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!inter%d_2);\n",inter[3*i][4],1-inter[3*i][5],inter[3*i][4],1-inter[3*i][5],i); strcat(s,s1);*/

            //direct representation
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | (LargeX[%d][%d]>0);\n",inter[3*i][0],inter[3*i][1],inter[3*i][0],inter[3*i][1],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | (LargeX[%d][%d]>0);\n",inter[3*i][4],inter[3*i][5],inter[3*i][4],inter[3*i][5],inter[3*i][0],inter[3*i][1]); strcat(s,s1);
            //COLLAPSE
            //sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!(LargeX[%d][%d]>0));\n",inter[3*i][0],1-inter[3*i][1],inter[3*i][0],1-inter[3*i][1],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            //sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!(LargeX[%d][%d]>0));\n",inter[3*i][4],1-inter[3*i][5],inter[3*i][4],1-inter[3*i][5],inter[3*i][0],inter[3*i][1]); strcat(s,s1);
        }else{
            /*sprintf(s1,"\tone_bit_t inter%d_0 = (LargeX[%d][%d]>0)&(LargeX[%d][%d]>0);\n",i,inter[3*i][2],inter[3*i][3],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tone_bit_t inter%d_1 = (LargeX[%d][%d]>0)&(LargeX[%d][%d]>0);\n",i,inter[3*i][0],inter[3*i][1],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tone_bit_t inter%d_2 = (LargeX[%d][%d]>0)&(LargeX[%d][%d]>0);\n",i,inter[3*i][0],inter[3*i][1],inter[3*i][2],inter[3*i][3]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | inter%d_0;\n",inter[3*i][0],inter[3*i][1],inter[3*i][0],inter[3*i][1],i); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | inter%d_1;\n",inter[3*i][2],inter[3*i][3],inter[3*i][2],inter[3*i][3],i); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | inter%d_2;\n",inter[3*i][4],inter[3*i][5],inter[3*i][4],inter[3*i][5],i); strcat(s,s1);
            //COLLAPSE
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!inter%d_0);\n",inter[3*i][0],1-inter[3*i][1],inter[3*i][0],1-inter[3*i][1],i); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!inter%d_1);\n",inter[3*i][2],1-inter[3*i][3],inter[3*i][2],1-inter[3*i][3],i); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & (!inter%d_2);\n",inter[3*i][4],1-inter[3*i][5],inter[3*i][4],1-inter[3*i][5],i); strcat(s,s1);*/

            //direct representation
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | ((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",inter[3*i][0],inter[3*i][1],inter[3*i][0],inter[3*i][1],inter[3*i][2],inter[3*i][3],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | ((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",inter[3*i][2],inter[3*i][3],inter[3*i][2],inter[3*i][3],inter[3*i][0],inter[3*i][1],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | ((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",inter[3*i][4],inter[3*i][5],inter[3*i][4],inter[3*i][5],inter[3*i][0],inter[3*i][1],inter[3*i][2],inter[3*i][3]); strcat(s,s1);
            //COLLAPSE
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & !((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",inter[3*i][0],1-inter[3*i][1],inter[3*i][0],1-inter[3*i][1],inter[3*i][2],inter[3*i][3],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & !((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",inter[3*i][2],1-inter[3*i][3],inter[3*i][2],1-inter[3*i][3],inter[3*i][0],inter[3*i][1],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] & !((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",inter[3*i][4],1-inter[3*i][5],inter[3*i][4],1-inter[3*i][5],inter[3*i][0],inter[3*i][1],inter[3*i][2],inter[3*i][3]); strcat(s,s1);

            //sprintf(s1,"\tL[%d][%d] = L[%d][%d] | (!((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0)));\n",inter[3*i][0],1-inter[3*i][1],inter[3*i][0],1-inter[3*i][1],inter[3*i][2],inter[3*i][3],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            //sprintf(s1,"\tL[%d][%d] = L[%d][%d] | (!((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0)));\n",inter[3*i][2],1-inter[3*i][3],inter[3*i][2],1-inter[3*i][3],inter[3*i][0],inter[3*i][1],inter[3*i][4],inter[3*i][5]); strcat(s,s1);
            //sprintf(s1,"\tL[%d][%d] = L[%d][%d] | (!((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0)));\n",inter[3*i][4],1-inter[3*i][5],inter[3*i][4],1-inter[3*i][5],inter[3*i][0],inter[3*i][1],inter[3*i][2],inter[3*i][3]); strcat(s,s1);
        }
        fprintf(fp4,s);
    }
    //Write check CONTRA
    fprintf(fp4,"\t//Check CONTRA rules of all units\n");
    for(i=0;i<size_contra_new;i++){
        strcpy(s,"");
        if(contra_new[i][6]==0){
            sprintf(s1,"\tone_bit_t contra%d = (LargeX[%d][%d]>0)&(LargeX[%d][%d]>0)&(LargeX[%d][%d]>0);\n",i,contra_new[i][0],contra_new[i][1],contra_new[i][2],contra_new[i][3],contra_new[i][4],contra_new[i][5]);
            strcat(s,s1);
            /*sprintf(s1,"\tL[%d][%d] = L[%d][%d] | ((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",contra_new[i][0],contra_new[i][1],contra_new[i][0],contra_new[i][1],contra_new[i][2],contra_new[i][3],contra_new[i][4],contra_new[i][5]);
            strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | ((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",contra_new[i][2],contra_new[i][3],contra_new[i][2],contra_new[i][3],contra_new[i][0],contra_new[i][1],contra_new[i][4],contra_new[i][5]);
            strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | ((LargeX[%d][%d]>0)&(LargeX[%d][%d]>0));\n",contra_new[i][4],contra_new[i][5],contra_new[i][4],contra_new[i][5],contra_new[i][0],contra_new[i][1],contra_new[i][2],contra_new[i][3]);
            strcat(s,s1);*/
        }else{
            sprintf(s1,"\tone_bit_t contra%d = (LargeX[%d][%d]>0)&(LargeX[%d][%d]>0)&",i,contra_new[i][0],contra_new[i][1],contra_new[i][2],contra_new[i][3]);
            strcat(s,s1);
            sprintf(s1,"(LargeX[%d][%d]>0)&(LargeX[%d][%d]>0);\n",contra_new[i][4],contra_new[i][5],contra_new[i][6],contra_new[i][7]);
            strcat(s,s1);
            sprintf(s1,"\tL[%d][%d] = L[%d][%d] | contra%d;\n",contra_new[i][6],contra_new[i][7],contra_new[i][6],contra_new[i][7],i);
            strcat(s,s1);
        }
        sprintf(s1,"\tL[%d][%d] = L[%d][%d] | contra%d;\n",contra_new[i][0],contra_new[i][1],contra_new[i][0],contra_new[i][1],i);
        strcat(s,s1);
        sprintf(s1,"\tL[%d][%d] = L[%d][%d] | contra%d;\n",contra_new[i][2],contra_new[i][3],contra_new[i][2],contra_new[i][3],i);
        strcat(s,s1);
        sprintf(s1,"\tL[%d][%d] = L[%d][%d] | contra%d;\n",contra_new[i][4],contra_new[i][5],contra_new[i][4],contra_new[i][5],i);
        strcat(s,s1);

        fprintf(fp4,s);
    }
    fprintf(fp4,"}");
    fclose(fp4);
    printf("created update rules L.\n");

}


