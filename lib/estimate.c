/* ----------------------------------------------------------------------
 * This program reads a data sample from a text file in the format
 *                         one data point per line 
 * and calculates an interval estimate for the mean of that (unknown) much 
 * larger set of data from which this sample was drawn.  The data can be 
 * either discrete or continuous.  A compiled version of this program 
 * supports redirection and can used just like program uvs.c. 
 * 
 * Name              : estimate.c  (Interval Estimation) 
 * Author            : Steve Park & Dave Geyer 
 * Language          : ANSI C 
 * Latest Revision   : 11-16-98 
 * ----------------------------------------------------------------------
 */

#include <math.h>
#include <stdio.h>
#include "rvms.h"

#define LOC 0.95                       /* level of confidence,        */ 
                                       /* use 0.95 for 95% confidence */

  int main(void)
{
  long   n    = 0;                     /* counts data points */
  double sum  = 0.0;
  double mean = 0.0;
  double data;
  double stdev;
  double u, t, w;
  double diff;

  while (!feof(stdin)) {                 /* use Welford's one-pass method */
    scanf("%lf\n", &data);               /* to calculate the sample mean  */
    n++;                                 /* and standard deviation        */
    diff  = data - mean;
    sum  += diff * diff * (n - 1.0) / n;
    mean += diff / n;
  }
  stdev  = sqrt(sum / n);

  if (n > 1) {
    u = 1.0 - 0.5 * (1.0 - LOC);              /* interval parameter  */
    t = idfStudent(n - 1, u);                 /* critical value of t */
    w = t * stdev / sqrt(n - 1);              /* interval half width */
    printf("\nbased upon %ld data points", n);
    printf(" and with %d%% confidence\n", (int) (100.0 * LOC + 0.5));
    printf("the expected value is in the interval");
    printf("%10.2f +/- %6.2f\n", mean, w);
  }
  else
    printf("ERROR - insufficient data\n");
  return (0);
}
