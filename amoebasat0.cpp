#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#define N_VARIABLE 225
#define N_CLAUSE 960
#define N_LITERAL 3

using namespace std;
const int MAX_N_STEP = 1000000;
const int MAX_CONTRA = 20000;
const int N_RUNS = 100;
const int EPSILON = 687194767;

int LargeX[N_VARIABLE+1][2];
int Y[N_VARIABLE+1][2];
int L[N_VARIABLE+1][2];
int Z[N_VARIABLE+1][2];
int f[N_CLAUSE][N_LITERAL];
bool x[N_VARIABLE+1];
bool x_fixed[N_VARIABLE+1];
int inter[3*N_CLAUSE][6];
int contra[MAX_CONTRA][8];
int size_inter;
int size_contra;

unsigned state[1];

int loadformula(char *filename);
bool sign_x(int x);
void generate_inter();
int survey_size_contra();
void generate_contra(int contra[MAX_CONTRA][8]);
void init();
void update_Y();
void update_Z();
int update_f();
void update_LargeX();
void update_x();
void update_L_intra();
void update_L_inter();
void update_L_contra();
void update_L_collapse();
int amoebasat();

int main() {
    char filename[128]="benchmarks/uf225-028.cnf";
    ofstream logfile;
    logfile.open("log.txt");
    logfile << filename << endl;

    int NStep, NStep_avg;
    cout << " N_VARIABLE = " << N_VARIABLE << ", N_CLAUSE = " << N_CLAUSE << endl;

    LargeX[0][0] = 0;

    if (!loadformula(filename)) {
        return 1;
    }

    generate_inter();
    size_contra = survey_size_contra();
    generate_contra(contra);
    NStep_avg = 0;
    srand(time(NULL));
    int i;
    for(i=0;i<N_RUNS;i++){
        state[0] = rand();
        init();
        cout <<"Run " << i+1 << ": seed = " << state[0] << endl;
        NStep = amoebasat();
        NStep_avg+=NStep;
        logfile << "Run " << i+1 <<": Found after " << NStep << " steps" << endl;
        for(int k=1;k<=N_VARIABLE;k++) logfile << x[k];
        logfile << endl;
    }
    cout << "Average: " << NStep_avg/i << " steps" << endl;
    logfile << "Average: " << NStep_avg/i << " steps" << endl;
    logfile.close();
    return 0;
}

int amoebasat(){
    int NStep;
    for(NStep=1;NStep<MAX_N_STEP;NStep++){
        int i;
        update_L_intra();
        update_L_inter();
        update_L_contra();
        update_L_collapse();
        update_Z();
        //update_Y();
        update_LargeX();
        update_x(); //cout <<"ok" <<endl;

        if(update_f()){
            cout<<"Found: "<<NStep<<" iterations ";
            for(i=1;i<=N_VARIABLE;i++) {
                cout<<x[i];
            }
            cout << endl;
            return NStep;
        }
    }
    printf("\nNot found.");

    return NStep;
}

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
        //printf("%s ", filename);
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
                        cout << "x_" << xi << " fixed = " << (int) x[xi] << endl;
                    }
                }
                clause_id++;
            }
        }
        fclose(fp);
    }
    return 1;
}
bool sign_x(int x){
    if(x>=0) return 0;
    else return 1;
}

void generate_inter(){
    int i;
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
    }
    printf("Size inter = %d\n", size_inter);
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
    //printf("Size contra survey = %d\n", size_contra);
    return size_contra;
}
void generate_contra(int contra[MAX_CONTRA][8]){
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
                    size_contra0++;
                }else{
                    contra1[size_contra1][0] = inter[j][0];
                    contra1[size_contra1][1] = inter[j][1];
                    contra1[size_contra1][2] = inter[j][2];
                    contra1[size_contra1][3] = inter[j][3];
                    size_contra1++;
                }
            }
        }
        for(j=0; j<size_contra0; j++){
            for(k=0; k<size_contra1; k++){
                //clear redundant lines from contra: if there's a line containing 2 counterparts of a unit, for ex 8 0 8 1, the rule is never contradicted, so no need to check
                if(contra0[j][0]==contra1[k][0] && contra0[j][1]==(1-contra1[k][1]) ) {
                    continue;
                }
                else if(contra0[j][0]==contra1[k][2] && contra0[j][1]==(1-contra1[k][3]) ) {
                    continue;
                }
                else if(contra0[j][2]==contra1[k][0] && contra0[j][3]==(1-contra1[k][1]) ) {
                    continue;
                }
                else if(contra0[j][2]==contra1[k][2] && contra0[j][3]==(1-contra1[k][3]) ) {
                    continue;
                }
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
    size_contra=counter;
    cout << "Size contra = " << size_contra << endl;
    
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
        //x[i]=0;
        for(j=0;j<2;j++){
            LargeX[i][j]=0;
            L[i][j]=0;
            while(Z[i][j]==0||Z[i][j]==0x40000000UL){
                Z[i][j] = xorshift32(state)>>1;
            }
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
        if(x_fixed[i]<1 && LargeX[i][0]==1 && LargeX[i][1]<=0){
            x[i] = 0;
        }else if(x_fixed[i]<1 && LargeX[i][1]==1 && LargeX[i][0]<=0){
            x[i] = 1;
        }
        //printf("x[%d]=%d \t", i, x[i]);
    }
}
void update_Y(){
    int i, j;
    for(i=1;i<=N_VARIABLE;i++){
        for(j=0; j<2; j++){
            if(L[i][j]==1){
                Y[i][j] = 0;
            }
            else if(L[i][j]==0){
                int sub = 2147483647 - EPSILON - Z[i][j] + 1;
                Y[i][j] = sign_x(-sub);
            }
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
                int sub = 2147483647 - EPSILON - Z[i][j] +1;
                //if Z > 1- epsilon, X expands; otherwise shrink
                if(sub>0 && LargeX[i][j]<1){
                    LargeX[i][j]++;
                }else if(sub<0 && LargeX[i][j]>-1){
                    LargeX[i][j]--;
                }
            }else if(L[i][j]==2) LargeX[i][j]==-1;
        }
    }
}
int update_f(){
    int clause_id;
    int f_val = 1;
    for(clause_id=0; clause_id<N_CLAUSE; clause_id++){
        int id1, id2, id3;
        bool c1, c2, c3;
        id1 = f[clause_id][0];
        id2 = f[clause_id][1];
        id3 = f[clause_id][2];
        if(id2==0)continue;
        c1 = sign_x(id1) ^ x[abs(id1)];
        c2 = sign_x(id2) ^ x[abs(id2)];
        if(id3==0)c3=0;
        else c3 = sign_x(id3) ^ x[abs(id3)];

        f_val = f_val && (c1 || c2 || c3);
        if (f_val==0) {
            //printf("unsat clause id = %d: %d %d %d \n",clause_id, id1, id2, id3);
            return f_val;
        }
    }
    return f_val;
}
void update_L_intra(){
    int i,j;
    for(i=1;i<=N_VARIABLE;i++){
    	L[i][0] = (LargeX[i][1]>0);
    	L[i][1] = (LargeX[i][0]>0);
    }
}
void update_L_inter(){
    int i;
    for(i=0;i<3*N_CLAUSE;i++){
        //if(inter[i][0]==0 && inter[i][2]==0) continue;
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
    }
}

void update_L_collapse(){
    int i;
    for(i=0;i<3*N_CLAUSE;i++){
        //if(inter[i][0]==0 && inter[i][2]==0) continue;
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
	    //COLLAPSE
	    L[ inter[i][4] ][ 1-inter[i][5] ] = L[ inter[i][4] ][ 1-inter[i][5] ] & (!inter1);
    }
}

void update_L_contra(){
    int i;
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

        L[ contra[i][0] ][ contra[i][1] ] = L[ contra[i][0] ][ contra[i][1] ] | contra1;
        L[ contra[i][2] ][ contra[i][3] ] = L[ contra[i][2] ][ contra[i][3] ] | contra1;
        L[ contra[i][4] ][ contra[i][5] ] = L[ contra[i][4] ][ contra[i][5] ] | contra1;
        L[ contra[i][6] ][ contra[i][7] ] = L[ contra[i][6] ][ contra[i][7] ] | contra1;

    }
}