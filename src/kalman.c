/* Matrix math. */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kalman.h"

/* This could be reduced to a single malloc if it mattered. */
Matrix alloc_matrix(int rows, int cols) {
  Matrix m;
  m.rows = rows;
  m.cols = cols;
  m.data = (double**) malloc(sizeof(double*) * m.rows);
  for (int i = 0; i < m.rows; ++i) {
    m.data[i] = (double*) malloc(sizeof(double) * m.cols);
    assert(m.data[i]);
    for (int j = 0; j < m.cols; ++j) {
      m.data[i][j] = 0.0;
    }
  }
  return m;
}

void free_matrix(Matrix m) {
  assert(m.data != NULL);
  for (int i = 0; i < m.rows; ++i) {
    free(m.data[i]);
  }
  free(m.data);
}

void set_matrix(Matrix m, ...) {
  va_list ap;
  va_start(ap, m);
  
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      m.data[i][j] = va_arg(ap, double);
    }
  }
  
  va_end(ap);
}

void set_identity_matrix(Matrix m) {
  assert(m.rows == m.cols);
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      if (i == j) {
	m.data[i][j] = 1.0;
      } else {
	m.data[i][j] = 0.0;
      }
    }
  }
}

void copy_matrix(Matrix source, Matrix destination) {
  assert(source.rows == destination.rows);
  assert(source.cols == destination.cols);
  for (int i = 0; i < source.rows; ++i) {
    for (int j = 0; j < source.cols; ++j) {
      destination.data[i][j] = source.data[i][j];
    }
  }
}

void print_matrix(Matrix m) {
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      if (j > 0) {
	printf(" ");
      }
      printf("%6.2f", m.data[i][j]);
    }
    printf("\n");
  }
}

void add_matrix(Matrix a, Matrix b, Matrix c) {
  assert(a.rows == b.rows);
  assert(a.rows == c.rows);
  assert(a.cols == b.cols);
  assert(a.cols == c.cols);
  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < a.cols; ++j) {
      c.data[i][j] = a.data[i][j] + b.data[i][j];
    }
  }
}

void subtract_matrix(Matrix a, Matrix b, Matrix c) {
  assert(a.rows == b.rows);
  assert(a.rows == c.rows);
  assert(a.cols == b.cols);
  assert(a.cols == c.cols);
  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < a.cols; ++j) {
      c.data[i][j] = a.data[i][j] - b.data[i][j];
    }
  }
}

void subtract_from_identity_matrix(Matrix a) {
  assert(a.rows == a.cols);
  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < a.cols; ++j) {
      if (i == j) {
	a.data[i][j] = 1.0 - a.data[i][j];
      } else {
	a.data[i][j] = 0.0 - a.data[i][j];
      }
    }
  }
}

void multiply_matrix(Matrix a, Matrix b, Matrix c) {
  assert(a.cols == b.rows);
  assert(a.rows == c.rows);
  assert(b.cols == c.cols);
  for (int i = 0; i < c.rows; ++i) {
    for (int j = 0; j < c.cols; ++j) {
      /* Calculate element c.data[i][j] via a dot product of one row of a
	 with one column of b */
      c.data[i][j] = 0.0;
      for (int k = 0; k < a.cols; ++k) {
	c.data[i][j] += a.data[i][k] * b.data[k][j];
      }
    }
  }
}

/* This is multiplying a by b-tranpose so it is like multiply_matrix
   but references to b reverse rows and cols. */
void multiply_by_transpose_matrix(Matrix a, Matrix b, Matrix c) {
  assert(a.cols == b.cols);
  assert(a.rows == c.rows);
  assert(b.rows == c.cols);
  for (int i = 0; i < c.rows; ++i) {
    for (int j = 0; j < c.cols; ++j) {
      /* Calculate element c.data[i][j] via a dot product of one row of a
	 with one row of b */
      c.data[i][j] = 0.0;
      for (int k = 0; k < a.cols; ++k) {
	c.data[i][j] += a.data[i][k] * b.data[j][k];
      }
    }
  }
}

void transpose_matrix(Matrix input, Matrix output) {
  assert(input.rows == output.cols);
  assert(input.cols == output.rows);
  for (int i = 0; i < input.rows; ++i) {
    for (int j = 0; j < input.cols; ++j) {
      output.data[j][i] = input.data[i][j];
    }
  }
}

int equal_matrix(Matrix a, Matrix b, double tolerance) {
  assert(a.rows == b.rows);
  assert(a.cols == b.cols);
  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < a.cols; ++j) {
      if (abs((int) (a.data[i][j] - b.data[i][j])) > tolerance) {
	return 0;
      }
    }
  }
  return 1;
}

void scale_matrix(Matrix m, double scalar) {
  assert(scalar != 0.0);
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      m.data[i][j] *= scalar;
    }
  }
}

void swap_rows(Matrix m, int r1, int r2) {
  assert(r1 != r2);
  double* tmp = m.data[r1];
  m.data[r1] = m.data[r2];
  m.data[r2] = tmp;
}

void scale_row(Matrix m, int r, double scalar) {
  assert(scalar != 0.0);
  for (int i = 0; i < m.cols; ++i) {
    m.data[r][i] *= scalar;
  }
}

/* Add scalar * row r2 to row r1. */
void shear_row(Matrix m, int r1, int r2, double scalar) {
  assert(r1 != r2);
  for (int i = 0; i < m.cols; ++i) {
    m.data[r1][i] += scalar * m.data[r2][i];
  }
}

/* Uses Gauss-Jordan elimination.

   The elimination procedure works by applying elementary row
   operations to our input matrix until the input matrix is reduced to
   the identity matrix.
   Simultaneously, we apply the same elementary row operations to a
   separate identity matrix to produce the inverse matrix.
   If this makes no sense, read wikipedia on Gauss-Jordan elimination.
   
   This is not the fastest way to invert matrices, so this is quite
   possibly the bottleneck. */
int destructive_invert_matrix(Matrix input, Matrix output) {
  assert(input.rows == input.cols);
  assert(input.rows == output.rows);
  assert(input.rows == output.cols);

  set_identity_matrix(output);

  /* Convert input to the identity matrix via elementary row operations.
     The ith pass through this loop turns the element at i,i to a 1
     and turns all other elements in column i to a 0. */
  for (int i = 0; i < input.rows; ++i) {
    if (input.data[i][i] == 0.0) {
      /* We must swap rows to get a nonzero diagonal element. */
      int r;
      for (r = i + 1; r < input.rows; ++r) {
	if (input.data[r][i] != 0.0) {
	  break;
	}
      }
      if (r == input.rows) {
	/* Every remaining element in this column is zero, so this
	   matrix cannot be inverted. */
	return 0;
      }
      swap_rows(input, i, r);
      swap_rows(output, i, r);
    }

    /* Scale this row to ensure a 1 along the diagonal.
       We might need to worry about overflow from a huge scalar here. */
    double scalar = 1.0 / input.data[i][i];
    scale_row(input, i, scalar);
    scale_row(output, i, scalar);

    /* Zero out the other elements in this column. */
    for (int j = 0; j < input.rows; ++j) {
      if (i == j) {
	continue;
      }
      double shear_needed = -input.data[j][i];
      shear_row(input, j, i, shear_needed);
      shear_row(output, j, i, shear_needed);
    }
  }
  
  return 1;
}


/* Kalman filters. */
KalmanFilter alloc_filter(int state_dimension,
			  int observation_dimension) {
  KalmanFilter f;
  f.timestep = 0;
  f.state_dimension = state_dimension;
  f.observation_dimension = observation_dimension;

  f.state_transition = alloc_matrix(state_dimension,
				    state_dimension);
  f.observation_model = alloc_matrix(observation_dimension,
				     state_dimension);
  f.process_noise_covariance = alloc_matrix(state_dimension,
					    state_dimension);
  f.observation_noise_covariance = alloc_matrix(observation_dimension,
						observation_dimension);

  f.observation = alloc_matrix(observation_dimension, 1);

  f.predicted_state = alloc_matrix(state_dimension, 1);
  f.predicted_estimate_covariance = alloc_matrix(state_dimension,
						 state_dimension);
  f.innovation = alloc_matrix(observation_dimension, 1);
  f.innovation_covariance = alloc_matrix(observation_dimension,
					 observation_dimension);
  f.inverse_innovation_covariance = alloc_matrix(observation_dimension,
						 observation_dimension);
  f.optimal_gain = alloc_matrix(state_dimension,
				observation_dimension);
  f.state_estimate = alloc_matrix(state_dimension, 1);
  f.estimate_covariance = alloc_matrix(state_dimension,
				       state_dimension);

  f.vertical_scratch = alloc_matrix(state_dimension,
				    observation_dimension);
  f.small_square_scratch = alloc_matrix(observation_dimension,
					observation_dimension);
  f.big_square_scratch = alloc_matrix(state_dimension,
				      state_dimension);
  
  return f;
}

void free_filter(KalmanFilter f) { 
  free_matrix(f.state_transition);
  free_matrix(f.observation_model);
  free_matrix(f.process_noise_covariance);
  free_matrix(f.observation_noise_covariance);

  free_matrix(f.observation);

  free_matrix(f.predicted_state);
  free_matrix(f.predicted_estimate_covariance);
  free_matrix(f.innovation);
  free_matrix(f.innovation_covariance);
  free_matrix(f.inverse_innovation_covariance);
  free_matrix(f.optimal_gain);
  free_matrix(f.state_estimate);
  free_matrix(f.estimate_covariance);

  free_matrix(f.vertical_scratch);
  free_matrix(f.small_square_scratch);
  free_matrix(f.big_square_scratch);
}

void update(KalmanFilter f) {
  predict(f);
  estimate(f);
}

void predict(KalmanFilter f) {
  f.timestep++;

  /* Predict the state */
  multiply_matrix(f.state_transition, f.state_estimate,
		  f.predicted_state);

  /* Predict the state estimate covariance */
  multiply_matrix(f.state_transition, f.estimate_covariance,
		  f.big_square_scratch);
  multiply_by_transpose_matrix(f.big_square_scratch, f.state_transition,
			       f.predicted_estimate_covariance);
  add_matrix(f.predicted_estimate_covariance, f.process_noise_covariance,
	     f.predicted_estimate_covariance);
}

void estimate(KalmanFilter f) {
  /* Calculate innovation */
  multiply_matrix(f.observation_model, f.predicted_state,
		  f.innovation);
  subtract_matrix(f.observation, f.innovation,
		  f.innovation);

  /* Calculate innovation covariance */
  multiply_by_transpose_matrix(f.predicted_estimate_covariance,
			       f.observation_model,
			       f.vertical_scratch);
  multiply_matrix(f.observation_model, f.vertical_scratch,
		  f.innovation_covariance);
  add_matrix(f.innovation_covariance, f.observation_noise_covariance,
	     f.innovation_covariance);

  /* Invert the innovation covariance.
     Note: this destroys the innovation covariance.
     TODO: handle inversion failure intelligently. */
  destructive_invert_matrix(f.innovation_covariance,
			    f.inverse_innovation_covariance);
  
  /* Calculate the optimal Kalman gain.
     Note we still have a useful partial product in vertical scratch
     from the innovation covariance. */
  multiply_matrix(f.vertical_scratch, f.inverse_innovation_covariance,
		  f.optimal_gain);

  /* Estimate the state */
  multiply_matrix(f.optimal_gain, f.innovation,
		  f.state_estimate);
  add_matrix(f.state_estimate, f.predicted_state,
	     f.state_estimate);

  /* Estimate the state covariance */
  multiply_matrix(f.optimal_gain, f.observation_model,
		  f.big_square_scratch);
  subtract_from_identity_matrix(f.big_square_scratch);
  multiply_matrix(f.big_square_scratch, f.predicted_estimate_covariance,
		  f.estimate_covariance);
}


//static const double PI = 3.14159265;
//static const double EARTH_RADIUS_IN_MILES = 3963.1676;

KalmanFilter alloc_filter_velocity2d(double noise) {
  /* The state model has four dimensions:
     x, y, x', y'
     Each time step we can only observe position, not velocity, so the
     observation vector has only two dimensions.
  */
  KalmanFilter f = alloc_filter(4, 2);

  /* Assuming the axes are rectilinear does not work well at the
     poles, but it has the bonus that we don't need to convert between
     lat/long and more rectangular coordinates. The slight inaccuracy
     of our physics model is not too important.
   */
//  double v2p = 0.001;
  set_identity_matrix(f.state_transition);
  set_seconds_per_timestep(f, 1.0);
	     
  /* We observe (x, y) in each time step */
  set_matrix(f.observation_model,
	     1.0, 0.0, 0.0, 0.0,
	     0.0, 1.0, 0.0, 0.0);

  /* Noise in the world. */
  double pos = 0.000001;
  set_matrix(f.process_noise_covariance,
	     pos, 0.0, 0.0, 0.0,
	     0.0, pos, 0.0, 0.0,
	     0.0, 0.0, 1.0, 0.0,
	     0.0, 0.0, 0.0, 1.0);

  /* Noise in our observation */
  set_matrix(f.observation_noise_covariance,
	     pos * noise, 0.0,
	     0.0, pos * noise);

  /* The start position is totally unknown, so give a high variance */
  set_matrix(f.state_estimate, 0.0, 0.0, 0.0, 0.0);
  set_identity_matrix(f.estimate_covariance);
  double trillion = 1000.0 * 1000.0 * 1000.0 * 1000.0;
  scale_matrix(f.estimate_covariance, trillion);

  return f;
}


/* The position units are in thousandths of latitude and longitude.
   The velocity units are in thousandths of position units per second.

   So if there is one second per timestep, a velocity of 1 will change
   the lat or long by 1 after a million timesteps.

   Thus a typical position is hundreds of thousands of units.
   A typical velocity is maybe ten.
*/
void set_seconds_per_timestep(KalmanFilter f,
			      double seconds_per_timestep) {
  /* unit_scaler accounts for the relation between position and
     velocity units */
  double unit_scaler = 0.001;
  f.state_transition.data[0][2] = unit_scaler * seconds_per_timestep;
  f.state_transition.data[1][3] = unit_scaler * seconds_per_timestep;
}

void update_velocity2d(KalmanFilter f, double lat, double lon,
		       double seconds_since_last_timestep) {
  set_seconds_per_timestep(f, seconds_since_last_timestep);
  set_matrix(f.observation, lat * 1000.0, lon * 1000.0);
  update(f);
}

void get_lat_long(KalmanFilter f, double* lat, double* lon) {
  *lat = f.state_estimate.data[0][0] / 1000.0;
  *lon = f.state_estimate.data[1][0] / 1000.0;
}

