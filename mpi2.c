#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define MASTER 0
#define FROM_MASTER 1
#define FROM_WORKER 2

#define NUM_USERS 1000
#define My NUM_USERS
#define NUM_MOVIES 1000
#define m 5

 #define F_x 640
 #define F_y 480

 int vec_sum[F_x];
 int Fi[F_x][F_y];
 int Fj[F_x][F_y];

 int matrix_user_log[NUM_USERS][NUM_MOVIES];
double matrix_corr[My][My];
int matrix_recom[My][m];


int     taskId,
        numTasks,
        numWorkers,
        sourceId,
        destId,
        currentWorker=0;

 MPI_Status status;

void initMPI(int argc, char **argv) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &taskId);
        MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
        numWorkers = numTasks-1;
}
void sendMatrixRanking() {
  int count = NUM_MOVIES * NUM_USERS;
  int index;
  int i;
  int w;
  for (i=0;i<numWorkers;i++) {
        w = nextWorker();
        //printf("Send-index=%d a %d\n",i,w);
    MPI_Send(&matrix_user_log[0][0], count, MPI_INT, w, FROM_MASTER, MPI_COMM_WORLD);
  }
    //empiezo a rotar los usuarios con sus indices
  for (i=0;i<NUM_USERS;i++) {
    w = nextWorker();
    MPI_Send(&i, 1, MPI_INT, w, FROM_MASTER, MPI_COMM_WORLD);
	        //printf("Send-index=%d a %d\n",i,w);
  }
  //printf("finalizando...\n");
  int fin=-1;
  for (i=1;i<=numWorkers;i++) {
    w = nextWorker();
        MPI_Send(&fin, 1, MPI_INT, w, FROM_MASTER, MPI_COMM_WORLD);
        //printf("finalizando el worker %d\n", w);
  }
}

int DoSequencial() {
int i;
        for (i=0;i<F_x;i++) {
                vec_sum[i] = processRow(i);
        }
}

int nextWorker() {
        if (currentWorker >= numWorkers)
                currentWorker = 0;
        currentWorker++;
        return currentWorker;
}

void generate_matrix_recom_row(int i){
  int j;
  int k;
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
  //print_matrix();
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

int get_user_highest_rated_movie(int user){
  int i;
  int maximum = -1;
  int maximum_index = 0;
  //printf("User:%d\n",user );
  for(i=0;i<NUM_MOVIES;i++){
    //printf("%d\n",matrix_user_log[user][i]);
    if(matrix_user_log[user][i] > maximum){
      maximum = matrix_user_log[user][i];
      maximum_index = i;
    }
  }
  return maximum_index;
  
  //printf("la película favorita de %d es %d\n",user,maximum);
}

void recommend_movies(int user){
  int i;
  int reco_movie;
  //get most similar users highest rated movies
  for(i=0 ; i<m ; i++){
    reco_movie = get_user_highest_rated_movie(matrix_recom[user][i]);
    printf("%d\n",reco_movie);
  }
}

void recvMatrixRanking() {
  int count = NUM_MOVIES * NUM_USERS;;
  int index = 0;
  int result;
  
	MPI_Recv(&matrix_user_log[0][0], count, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,&status);
	
	  while (index != -1) {
        MPI_Recv(&index, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD,&status);
		count = NUM_USERS;
        if (index != -1) {
                processRow(index);
				//printf("termino procesamiento en %d\n", taskId);
				MPI_Send(&index, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD);
                MPI_Send(&matrix_corr[index][0], count, MPI_DOUBLE, MASTER, FROM_MASTER, MPI_COMM_WORLD);	
				generate_matrix_recom_row(index);
				MPI_Send(&matrix_recom[index][0], m, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD);
                //printf("enviados los resultados (task=%d) index=%d ind=%f match=%f\n\n", taskId,index, referencia[index][0], referencia[index][1]);
        }
  }
	//result = processRow(index);
}
int processRow(int index) {
    int j;
    for ( j = 0; j < NUM_USERS; ++j )
    {
      matrix_corr[index][j] = pearson(matrix_user_log[index], matrix_user_log[j]);
    }
}

void recvResultsCorrelation() {
  int count = NUM_USERS;
  int i, index,w;
  currentWorker=0;
  for (i=0;i<count;i++) {
        w = nextWorker();
        //printf("recvResults(%d) waiting data from %d\n", taskId,w);
    MPI_Recv(&index, 1, MPI_INT, w, FROM_MASTER, MPI_COMM_WORLD, &status);
        if (index != -1) {
                MPI_Recv(&matrix_corr[index][0], count, MPI_DOUBLE, w, FROM_MASTER, MPI_COMM_WORLD,&status);
                //printf("recvResults(%d) index=%d correlation=%f\n", taskId,index,matrix_corr[index][0]);
				MPI_Recv(&matrix_recom[index][0], m, MPI_INT, w, FROM_MASTER, MPI_COMM_WORLD,&status);
				//printf("recvResults(%d) index=%d recomen=%d\n", taskId,index,matrix_recom[index][0]);
        }
  }
}

void fillMatrix() {
        int i,j;
        int val_i, val_j;
        int prob;
        for (i=0;i<F_x;i++)
                for (j=0;j<F_y;j++) {
                        val_i = rand()%256;
                        val_j = rand()%256;
                        Fi[i][j] = val_i;
                        Fj[i][j] = val_j;
                }
 }
 double start;
 void main(int argc, char **argv) {
        initMPI(argc, argv);
		start = MPI_Wtime();
		if (taskId == MASTER) {
				printf("Matriz tomada desde el fichero de texto:\n");
  FILE *user_log = fopen( "usuarios-peliculas.txt", "r" );

  if (user_log == NULL)
  {
    fprintf(stderr, "%s\n", "Error abriendo usuarios-peliculas.txt");
    exit(EXIT_FAILURE);
  }

  fill_user_logs(user_log);
  fclose(user_log);
  
  sendMatrixRanking();
  recvResultsCorrelation();
  
    printf("Matriz de Correlacion:\n");
  //print_matrix_corr();
  
    printf("Matriz de recomendación:\n");
 // print_matrix_recom();
  
    //recomend movies to a specific user
  int user = rand() % 10;

  printf("recomendaciones para el usuario %d:\n",user);

  recommend_movies(user);
  
  printf("Processing time: %lf\n", MPI_Wtime()-start);
        } else {
		recvMatrixRanking();
		//printf("recibida matriz");
		//print_matrix();
        }
		
  
       /*
	   start = MPI_Wtime();
        if (taskId == MASTER) {
                fillMatrix();
                sendRows();
                recvResults();
                printf("Processing time: %lf\n", MPI_Wtime()-start);
        } else {
                recvRows();
        }*/
        MPI_Finalize();
 }
