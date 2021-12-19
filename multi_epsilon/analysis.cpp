#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#define N_LITERAL 3
using namespace std;

int N_VARIABLE;
int N_CLAUSE;
int **f;
char *x;
char *x_fixed;
int **inter;
int size_inter;
int size_contra;
int size_contra_new;
int **contra;
int **contra_new;
int **conn; //connections# of all variables

void getParams(char* filename);
int loadformula(char *filename);
int sign_x(int x);
void generate_inter();
int survey_size_contra();
void generate_contra();
void create_local_rules(int **inter, int **contra_new);


FILE *fp1; //z
FILE *fp2; //inter
FILE *fp3; //contra
FILE *fp4; //more info

int main(int argc, char** argv) {
    char* filename = argv[1];

    getParams(filename);    
    cout << " N_VARIABLE = " << N_VARIABLE << ", N_CLAUSE = " << N_CLAUSE << endl;

    //memory allocation f[N_CLAUSE][3];
    f=(int**) malloc(sizeof(int*) * N_CLAUSE);
    for(int i=0;i<N_CLAUSE;i++){
        f[i]=(int*) calloc(N_LITERAL,sizeof(int));
    }
    x = (char*) malloc(N_VARIABLE+1); 
    x_fixed = (char*) malloc(N_VARIABLE+1);
    
    inter=(int**)malloc(sizeof(int*) * N_CLAUSE * N_LITERAL);
    for(int i=0;i<N_CLAUSE * N_LITERAL;i++){
        inter[i]=(int*)malloc(sizeof(int) * 2 * N_LITERAL);
    }
    conn=(int**)malloc(sizeof(int*) * (N_VARIABLE+1));
    for(int i=0;i<= N_VARIABLE;i++){
        conn[i]=(int*)malloc(sizeof(int) * 2);
    }

    if (!loadformula(filename)) {
        return 1;
    }
    //system("mkdir rules");
    generate_inter(); 
    size_contra = survey_size_contra();
    contra=(int**)malloc(sizeof(int*) * size_contra);
    for(int i=0;i<size_contra;i++){
        contra[i]=(int*)malloc(sizeof(int) * 8);
    }
    contra_new=(int**)malloc(sizeof(int*) * size_contra);
    for(int i=0;i<size_contra;i++){
        contra_new[i]=(int*)malloc(sizeof(int) * 8);
    }
    generate_contra(); 
    create_local_rules(inter, contra_new);
    
    return 0;
}
void getParams(char *filename) {   
    N_VARIABLE = 0; 
    int clause_id = 0;
    FILE *fp;
    fp = fopen(filename, "r");
    char str[255];
    char *token;
    int j; // i = order of token; j = 0,1,2 -> 3 literals per clause
    if (fp == NULL) {
        printf("File not found!");
    }
    else {
        printf("%s :", filename);
        while (!feof(fp)) {
            fgets(str, 255, fp);
            // get N_LITERAL, N_CLAUSE
            if (strstr(str, "p cnf")) {
                token = strtok(str, " ,\t\n"); //get p
                token = strtok(NULL," ,\t\n"); //get cnf
                token = strtok(NULL," ,\t\n");
                N_VARIABLE = atoi(token);
                token = strtok(NULL," ,\t\n");
                N_CLAUSE = atoi(token);
                return;
            }
            //skip reading first lines begin with c
            else if (strstr(str, "c"))
                continue;
            // get Clause parameters
            else {
                token = strtok(str, " ,\t\n");
                if (strstr(str, "%"))
                    break;                    
                int lit = abs(atoi(token));
                if(lit>N_VARIABLE) N_VARIABLE = lit;
                for (j = 1; j < N_LITERAL; j++) {
                    token = strtok(NULL, " ,\t\n");
                    lit = abs(atoi(token));
                    if(lit>N_VARIABLE) N_VARIABLE = lit;
                }
            }
            clause_id++;            
        }
        N_CLAUSE = clause_id;
        fclose(fp);
    }
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
                        //printf("x%d fixed = %d \n", xi, x[xi]);
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
int sign_x(int x){
    if(x>=0) return 0;
    else return 1;
}

void generate_inter(){
    int i;
    
    fp2 = fopen("./f.h", "w+");
    fp3 = fopen("./f_sign.h", "w+");
    //fp4 = fopen("f_tmp.txt", "w+");
    
    fprintf(fp2,"f_t f[N_CLAUSE][3]={\n");
    fprintf(fp3,"one_bit_t f_sign[N_CLAUSE][3]={\n");
    size_inter=0;
    for(i=0; i<N_CLAUSE; i++){
        //-------- generate inter set size 3x
        int interQ, interQ_abs, interP1, interP1_abs, interP2, interP2_abs;
        if(f[i][1]==0) {
			printf("\nClause%d:\t%d\t%d\t%d",i,f[i][0],f[i][1],f[i][2]);
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
        if(i==(N_CLAUSE-1)) fprintf(fp2, "%d, %d, %d};", interQ, interP1, interP2);
        else fprintf(fp2, "%d, %d, %d,\n", interQ, interP1, interP2);

        if(i==(N_CLAUSE-1)) fprintf(fp3, "%d, %d, %d};", interQ_abs, interP1_abs, interP2_abs);
        else fprintf(fp3, "%d, %d, %d,\n", interQ_abs, interP1_abs, interP2_abs);
    }
    printf("Size inter = %d\n", size_inter);
    fclose(fp2);
    fclose(fp3);
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
void generate_contra(){
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
    
    size_contra_new=size_contra;
}
//create local rules and local modules
void create_local_rules(int **inter, int **contra_new){
    int i, j; //unit index
    int k, l, m, n;
    int count_rules[N_VARIABLE+1][2];
    char s[1000000]; //temporary text
    char s1[100]; //text for 1 line
    fp4 = fopen("local_rules.txt", "w+");
    fp3 = fopen("conn.txt","w+");
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
            //printf("unit(%d,%d) %d\n",i,j,connection_count);
            fprintf(fp3,"%d\n",connection_count);
            //write rules into file
            fprintf(fp4, "int unit%d_%d[%d][6]={\n", i, j,count_rules[i][j]);
            fprintf(fp4,s);
        }
    }
    fclose(fp4);
    fclose(fp3);

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