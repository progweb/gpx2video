#ifndef __KALMAN_H__
#define __KALMAN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /* Dimensions */
  int rows;
  int cols;

  /* Contents of the matrix */
  double** data;
} Matrix;

/* Allocate memory for a new matrix.
   Zeros out the matrix.
   Assert-fails if we are out of memory.
*/
Matrix alloc_matrix(int rows, int cols);

/* Free memory for a matrix. */
void free_matrix(Matrix m);

/* Set values of a matrix, row by row. */
void set_matrix(Matrix m, ...);

/* Turn m into an identity matrix. */
void set_identity_matrix(Matrix m);

/* Copy a matrix. */
void copy_matrix(Matrix source, Matrix destination);

/* Pretty-print a matrix. */
void print_matrix(Matrix m);

/* Add matrices a and b and put the result in c. */
void add_matrix(Matrix a, Matrix b, Matrix c);

/* Subtract matrices a and b and put the result in c. */
void subtract_matrix(Matrix a, Matrix b, Matrix c);

/* Subtract from the identity matrix in place. */
void subtract_from_identity_matrix(Matrix a);

/* Multiply matrices a and b and put the result in c. */
void multiply_matrix(Matrix a, Matrix b, Matrix c);

/* Multiply matrix a by b-transpose and put the result in c. */
void multiply_by_transpose_matrix(Matrix a, Matrix b, Matrix c);

/* Transpose input and put the result in output. */
void transpose_matrix(Matrix input, Matrix output);

/* Whether two matrices are approximately equal. */
int equal_matrix(Matrix a, Matrix b, double tolerance);

/* Multiply a matrix by a scalar. */
void scale_matrix(Matrix m, double scalar);

/* Swap rows r1 and r2 of a matrix.
   This is one of the three "elementary row operations". */
void swap_rows(Matrix m, int r1, int r2);

/* Multiply row r of a matrix by a scalar.
   This is one of the three "elementary row operations". */
void scale_row(Matrix m, int r, double scalar);

/* Add a multiple of row r2 to row r1.
   Also known as a "shear" operation.
   This is one of the three "elementary row operations". */
void shear_row(Matrix m, int r1, int r2, double scalar);

/* Invert a square matrix.
   Returns whether the matrix is invertible.
   input is mutated as well by this routine. */
int destructive_invert_matrix(Matrix input, Matrix output);


/* Refer to http://en.wikipedia.org/wiki/Kalman_filter for
   mathematical details. The naming scheme is that variables get names
   that make sense, and are commented with their analog in
   the Wikipedia mathematical notation.
   This Kalman filter implementation does not support controlled
   input.
   (Like knowing which way the steering wheel in a car is turned and
   using that to inform the model.)
   Vectors are handled as n-by-1 matrices.
   TODO: comment on the dimension of the matrices */
typedef struct {
  /* k */
  int timestep;

  /* These parameters define the size of the matrices. */
  int state_dimension, observation_dimension;
  
  /* This group of matrices must be specified by the user. */
  /* F_k */
  Matrix state_transition;
  /* H_k */
  Matrix observation_model;
  /* Q_k */
  Matrix process_noise_covariance;
  /* R_k */
  Matrix observation_noise_covariance;

  /* The observation is modified by the user before every time step. */
  /* z_k */
  Matrix observation;
  
  /* This group of matrices are updated every time step by the filter. */
  /* x-hat_k|k-1 */
  Matrix predicted_state;
  /* P_k|k-1 */
  Matrix predicted_estimate_covariance;
  /* y-tilde_k */
  Matrix innovation;
  /* S_k */
  Matrix innovation_covariance;
  /* S_k^-1 */
  Matrix inverse_innovation_covariance;
  /* K_k */
  Matrix optimal_gain;
  /* x-hat_k|k */
  Matrix state_estimate;
  /* P_k|k */
  Matrix estimate_covariance;

  /* This group is used for meaningless intermediate calculations */
  Matrix vertical_scratch;
  Matrix small_square_scratch;
  Matrix big_square_scratch;
  
} KalmanFilter;

KalmanFilter alloc_filter(int state_dimension,
			  int observation_dimension);

void free_filter(KalmanFilter f);

/* Runs one timestep of prediction + estimation.

   Before each time step of running this, set f.observation to be the
   next time step's observation.

   Before the first step, define the model by setting:
   f.state_transition
   f.observation_model
   f.process_noise_covariance
   f.observation_noise_covariance

   It is also advisable to initialize with reasonable guesses for
   f.state_estimate
   f.estimate_covariance
*/
void update(KalmanFilter f);

/* Just the prediction phase of update. */
void predict(KalmanFilter f);
/* Just the estimation phase of update. */
void estimate(KalmanFilter f);



KalmanFilter alloc_filter_velocity2d(double noise);

void set_seconds_per_timestep(KalmanFilter f,
			      double seconds_per_timestep);

void update_velocity2d(KalmanFilter f, double lat, double lon,
		       double seconds_since_last_timestep);

void get_lat_long(KalmanFilter f, double* lat, double* lon);

#ifdef __cplusplus
}
#endif

#endif
