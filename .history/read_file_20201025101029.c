/* Title: Direct Matrix Calculations
Author: Ewan-James Thomas
Address: 35 Monkswell Road EX4 7AU
License: Public Domain
*/

static const char * VERSION = "0.4.0";
static const char * REV_DATE = "24-October-2020";

/*
 Date         Version  Comments
 ----         -------  --------
09-Oct-2020     0.0.1  Build start -> taking input from command line.
10-Oct-2020     0.0.2  Added "getopt" functionality, Trying to get_matrix...
11-Oct-2020     0.0.3  Continued work on get_matrix function
14-Oct-2020     0.0.4  Continued work on get_matrix function
16-Oct-2020     0.1.0  Code can now read in matrix and produce the frobenius norm
17-Oct-2020     0.2.0  Code can now display transpose of matrix to stdout
18-Oct-2020     0.2.1  Work started in implementing product function
21-Oct-2020     0.3.0  Code can now print the product of two matricies from command line
24-Oct-2020     0.4.0  Program can now find the determinant of a matrix
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#define MAX_FILE_LINE_SIZE 250
#define LINE_NUMBER 3
#define ITEMS_LINE 2
#define MAX_FILES 2

/* Matrix structure contains all necessary information and thus limits required function parameters*/
typedef struct
{   
    int rows;
    int cols;
    double *data;
} Matrix;
    

Matrix *read_from_file(const char *filename){
    
    char line[MAX_FILE_LINE_SIZE];
    char newLine[MAX_FILE_LINE_SIZE]; 
    
    FILE *file = fopen(filename,"r");
    
    if(!file){
        fprintf(stderr, "Error: Could not open the file '%s'.\n",filename);
        exit(1);
    }

    Matrix *m = (Matrix *) malloc(sizeof(Matrix));

     /* We will use rows and cols to malloc a matrix*/ 
    while(fgets(line, MAX_FILE_LINE_SIZE, file)){
        if(line[0]=='m'){
            int DimFound = sscanf(line, "%*s %d %d", &m->rows, &m->cols);
            if(DimFound != ITEMS_LINE){
                fprintf(stderr, "Unknown format: %s\n", line);
                break;
            }
        }
    }
    rewind(file);
    
    m->data = (double *) malloc(m->rows*m->cols*sizeof(double));

    if(!m){
        fprintf(stderr, "Memory Error\n");
        fclose(file);
        return NULL;
    }

    /* Skipping commented lines, we'll now assign values to the matrix which is maped onto a 1D array for the sake of speed */
    while(fgets(newLine, MAX_FILE_LINE_SIZE, file)){
        if(newLine[0] != '#' && newLine[0] != 'e'){
            for(size_t i = 0; i < m->rows; ++i){
                for (size_t j = 0; j < m->cols; ++j){
                    fscanf(file, "%lg", &m->data[(m->cols*i)+j]);
                } 
            }
        }
    }

    fclose(file);
    return m;

}

/* The frobenius norm is the sqrt of the sum of all elements squared */
double frobenius_norm(Matrix *matrix){
    printf("#Version = %s, Revision date = %s\n",VERSION, REV_DATE);
    double sum = 0.0;
    for(int i = 0; i<matrix->cols; i++){
        for(int j = 0; j < matrix->rows; j++){
            sum += matrix->data[matrix->cols*i + j]*matrix->data[matrix->cols*i + j];
            printf("%lg\n",sum);
        }
    }
    return sqrt(sum);
}

/* The transpose turns rows into columns and vice versa */
void transpose(Matrix *matrix){
    for(int i = 0; i < matrix->cols; i++){
        for(int j = 0; j < matrix->rows; j++){
            printf("%lg\t", matrix->data[matrix->cols*j+i]);/* Cols*j+i goes down the column first rather than across the row*/
        }
        printf("\n");
    }
}

/*The product of two matricies requires correct dimensionality. Only then can we proceed with multiplication */
void product(Matrix *matrix_1, Matrix *matrix_2){
    if(matrix_1->cols != matrix_2->rows){
        printf("Matrices are of the wrong dimension and thus cannot be multiplied.\n");
        return;
    }
    printf("matrix %d %d\n",matrix_1->rows, matrix_2->cols);
    for (size_t i = 0; i < matrix_1->rows; i++){
        for (size_t j = 0; j < matrix_2->cols; j++){
            double sum = 0.0;
            for(size_t k = 0; k < matrix_2->rows; k++){
                sum += (matrix_1->data[matrix_1->cols*i+k])*(matrix_2->data[matrix_2->cols*k+j]);
            }
            printf("%lg\t", sum);
        } printf("\n");
    }
}

/* The determinant requires a matrix to be split into submatricies */
/* This will continue until the size of the submatricies reaches the base case in the recursive loop */
double determinant(Matrix *matrix){
    if(matrix->rows != matrix->cols){
        printf("Error: Non-square matricies do not have a determinant");
        return -1;
    }

    double det = 0.0;
    int rank = matrix->rows;
    int c = 1;
    Matrix *submatrix = (Matrix *) malloc(sizeof(Matrix));
    submatrix->rows = rank-1; 
    submatrix->cols = rank-1;
    submatrix->data = (double *) malloc(submatrix->rows*submatrix->cols*sizeof(double));

    /* The base case */
    if(rank == 2){
        det = matrix->data[0]*matrix->data[3]-matrix->data[1]*matrix->data[2];
    }
    /* Create a submatrix that can be fed back into the function to reach the base case */
    else{
        for(int i = 0; i < rank; i++){
            int q = 0;
            int p = 0;
            for(int j =0; j < rank; j++){
                for(int k = 0; k < rank; k++){
                    if(k!=i && j!=0){ /*Always calculate determinant using top line elements as prefactors for simplicity*/
                        submatrix->data[(submatrix->cols*q)+p] = matrix->data[matrix->cols*j+k];
                        if(p < (rank - 2)){
                            p++;
                        }else{
                            p = 0;
                            q++;
                        }
                    }
                }
            }
        det = det + c * (matrix->data[i] * determinant(submatrix));
        c *= -1;
        }
    }
    return (det);
}

void adjoint(Matrix *matrix){
    if(matrix->rows != matrix->cols){
        printf("Error: Non-square matricies do not have an adjoint matrix");
        return;
    }

    int rank = matrix->rows;

    Matrix *cofactor = (Matrix *) malloc(sizeof(Matrix));
    cofactor->rows = matrix->rows;
    cofactor ->cols = matrix->cols;
    cofactor->data = (double *) malloc(cofactor->rows*cofactor->cols*sizeof(double));

    Matrix *submatrix = (Matrix *) malloc(sizeof(Matrix));
    submatrix->rows = matrix->rows-1;
    submatrix->cols = matrix->cols-1;
    submatrix->data = (double *) malloc(submatrix->rows*submatrix->cols*sizeof(double));

    /* Similar approach to find the adjoint as we have to find many mini-determinants*/
    for(int g = 0; g < rank; g++){ /*Extra for loop compared to the determinant as we have to go to visit elements not just top row*/
        for(int h = 0; h < rank; h++){

            int m = 0;
            int n = 0;

            for(int i = 0; i < rank; i++){
                for(int j = 0; j < rank; j++){
                    if(j!=h && i!=g){
                        submatrix->data[submatrix->cols*m+n] = matrix->data[matrix->cols*i+j];
                        /*printf("%lg\n", submatrix->data[submatrix->cols*m+n]);*/
                        if(n < (rank -2)){
                            n++;
                        }
                        else{
                            n=0;
                            m++;
                        }
                        printf("%lg\n",submatrix->data[4]);   
                    }
                }
            }
            cofactor->data[rank*g+h] = pow(-1, (g+h))*determinant(submatrix); /*Minus sign depends on location in the matrix*/
            
        }
    }
    /*transpose(cofactor);*/
    return;
}

int main(int argc, char **argv){
    
    Matrix *mats[MAX_FILES]; /*Array of matrix pointers*/
    
        if(argc == 3){
            mats[0] = read_from_file(argv[argc-1]);
        }

        if(argc == 4){
            mats[0] = read_from_file(argv[argc-2]);
            mats[1] = read_from_file(argv[argc-1]);

        }
    
    int option;

        /* Checks for flags from command line*/
    while ((option = getopt(argc, argv, "ftmdai")) != -1){
        switch(option){
            case 'f' :
                printf("#You want the frobenius norm\n %lg\n", frobenius_norm(mats[0]));
                break;
            case 't' :
                printf("#You want the transpose\nmatrix %d %d\n",mats[0]->cols, mats[0]->rows);
                transpose(mats[0]);
                break;
            case 'm' :
                printf("#You want to multiply two matricies\n");
                product(mats[0], mats[1]);
                break;
            case 'd' :
                printf("#You want the Determinant\n");
                printf("%lg\n", determinant(mats[0]));
                break;
            case 'a' :
                printf("#You want the adjoint\n");
                adjoint(mats[0]);
                break;
            case 'i' :
                printf("#You want the inverse\n");
                break;   
            default:
                printf("Error: Options '-%c' is not a valid input\n", optopt);
        }
    }

    return 0;

}
