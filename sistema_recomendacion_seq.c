#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_USERS 10
#define My NUM_USERS
#define NUM_MOVIES 10
#define m 5 


// ruby fill logs with random numbers
// 10.times { 10.times { print "#{rand.rand 5} " } ; puts "" }

int matrix_user_log[NUM_USERS][NUM_MOVIES];
double matrix_corr[My][My];
int matrix_recom[My][m];

void generate_matrix_recom(){
  int i;
  int j;
  int k;
  for ( i = 0; i < NUM_USERS; ++i ){
    for ( j = 0; j < m; ++j ){
      double maximum = -2;
      int maximum_index;
      for(k = 0;k < NUM_USERS; k++){
        if(matrix_corr[i][k] > maximum && i != k){
          maximum = matrix_corr[i][k];
          maximum_index = k;
        }
      }
      matrix_recom[i][j] = maximum_index;
      matrix_corr[i][maximum_index] = -2;
      //printf("el mayor índice fue %f del usuario %d\n",maximum,maximum_index);
    }
    //printf("--------------"); 
  }
}

void print_matrix_recom()
{
  int i;
  int j;
  for ( i = 0; i < NUM_USERS; ++i )
  {
    for ( j = 0; j < m; ++j )
    {
      printf("%d ", matrix_recom[i][j]);
    }
    printf("\n");
  }
}


void print_matrix()
{
  int i;
  int j;
  for ( i = 0; i < NUM_USERS; ++i )
  {
    for ( j = 0; j < NUM_MOVIES; ++j )
    {
      printf("%d ", matrix_user_log[i][j]);
    }
    printf("\n");
  }
}

void print_matrix_corr()
{
  int i;
  int j;
  for ( i = 0; i < NUM_USERS; ++i )
  {
    for ( j = 0; j < NUM_USERS; ++j )
    {
      printf("%f ", matrix_corr[i][j]);
    }
    printf("\n");
  }
}

int vector_sum(int vector [NUM_MOVIES])
{
  int sum = 0;
  int i;
  for ( i = 0; i < NUM_MOVIES; ++i )
  {
    sum += vector[i];
  }
  return sum;
}

int vector_pow(int vector [NUM_MOVIES])
{
  int sum = 0;
  int i;
  for ( i = 0; i < NUM_MOVIES; ++i )
  {
    sum += (vector[i] * vector[i]);
  }
  return sum;
}

void fill_user_logs(FILE *file)
{
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *rate_array = NULL;
  char *token = NULL;
  int rows = 0;
  int n;
  while ((read = getline(&line, &len, file)) != -1) {
    for ( n = 0; ; line = NULL, ++n ) {
      token = strtok_r(line, " ", &rate_array);
      if ( token == NULL ){
        break;
      }
      matrix_user_log[rows][n] = atoi(token);
    }
    ++rows;
  }

  if (line){
    free(line);
  }
  print_matrix();
}

double pearson(int user1 [NUM_MOVIES], int user2 [NUM_MOVIES])
{
  int i;
  int sumX, sumY, sumXPow, sumYPow, sumProd;
  int product[NUM_MOVIES];

  sumX = vector_sum(user1);
  sumY = vector_sum(user2);
  sumXPow = vector_pow(user1);
  sumYPow = vector_pow(user2);

  for ( i = 0; i < NUM_MOVIES; ++i )
  {
    product[i] = user1[i]*user2[i];
  }

  sumProd = vector_sum(product);

  double res = sumProd - ( sumX * sumY / (double) NUM_MOVIES );
  double den = pow(
    ( (double) ( sumXPow - ( sumX * sumX ) / (double) NUM_MOVIES ) *
    ( sumYPow - ( sumY * sumY ) / (double) NUM_MOVIES ) ), 0.5 );
  if ( den == 0.0 ) { return 0;}
  return res / den;
}

int main(int argc, char const *argv[])
{
  FILE *user_log = fopen( "usuarios-peliculas.txt", "r" );

  if (user_log == NULL)
  {
    fprintf(stderr, "%s\n", "Error abriendo usuarios-peliculas.txt");
    exit(EXIT_FAILURE);
  }

  fill_user_logs(user_log);
  fclose(user_log);
  int i, j;
  for ( i = 0; i < NUM_USERS; ++i )
  {
    for ( j = 0; j < NUM_USERS; ++j )
    {
      matrix_corr[i][j] = pearson(matrix_user_log[i], matrix_user_log[j]);
    }
  }
  printf("Matrix Correlacion:\n");
  print_matrix_corr();

  generate_matrix_recom();
  print_matrix_recom();

  return 0;
}
