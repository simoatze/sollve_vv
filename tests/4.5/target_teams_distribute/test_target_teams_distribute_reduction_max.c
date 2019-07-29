//===--- test_target_teams_distribute_reduction_max.c------------------------===//
//
// OpenMP API Version 4.5 Nov 2015
//
// This test uses the reduction clause on a target teams distribute directive,
// testing that the variable in the reduction clause is properly reduced using
// the max operator.
//
////===----------------------------------------------------------------------===//

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "ompvv.h"
#include <math.h>

#define N 1024

int test_max() {
  int a[N];
  int b[N];
  int errors = 0;
  int num_teams[N];
  int warned = 0;
  srand(1);

  for (int x = 0; x < N; ++x) {
    a[x] = (int) rand() / (double)(RAND_MAX / 100);
    b[x] = (int) rand() / (double)(RAND_MAX / 100);
    num_teams[x] = -x;
  }

  int result = 0;

#pragma omp target data map(tofrom: num_teams[0:N]) map(to: a[0:N], b[0:N])
  {
#pragma omp target teams distribute reduction(max:result) map(alloc: a[0:N], b[0:N], num_teams[0:N])
    for (int x = 0; x < N; ++x) {
      result = fmax(a[x] + b[x], result);
      num_teams[x] = omp_get_num_teams();
    }
  }

  int host_max = 0;

  for (int x = 0; x < N; ++x) {
    host_max = fmax(host_max, a[x] + b[x]);
  }

  for (int x = 1; x < N; ++x) {
    if (num_teams[x-1] != num_teams[x]) {
      OMPVV_WARNING("Kernel reported multiple numbers of teams.  Validity of testing of reduction clause cannot be guarunteed.");
      warned += 1;
    }
  }
  if ((num_teams[0] == 1) && (warned == 0)) {
    OMPVV_WARNING("Test operated with one team.  Reduction clause cannot be tested.");
  } else if ((num_teams[0] <= 0) && (warned == 0)) {
    OMPVV_WARNING("Test reported invalid number of teams.  Validity of testing of reduction clause cannot be guarunteed.")
      }

  OMPVV_TEST_AND_SET_VERBOSE(errors, result != host_max);
  return errors;
}

int main() {
  int total_errors = 0;
  OMPVV_TEST_AND_SET_VERBOSE(total_errors, test_max() != 0);
  OMPVV_REPORT_AND_RETURN(total_errors);
}