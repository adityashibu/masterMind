/*
  A C program to test the matching function (for master-mind) as implemented in matches.s

$ as  -o mm-matches.o mm-matches.s
$ gcc -c -o testm.o testm.c
$ gcc -o testm testm.o mm-matches.o
$ ./testm
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>

#define LENGTH 3
#define COLORS 3

#define NAN1 8
#define NAN2 9

const int seqlen = LENGTH;
const int seqmax = COLORS;

/* ********************************** */
/* take these fcts from master-mind.c */
/* ********************************** */

/* Show given sequence */
// void showSeq(int *seq)
// {
//   printf("Secret: ");
//   for (int i = 0; i < LENGTH; i++)
//   {
//     printf("%d ", seq[i]);
//   }
//   printf("\n");
// }

/* Parse an integer value as a list of digits of base MAX */
// void readSeq(int *seq, int val)
// {
//   int first = val / 100;
//   int second = (val % 100) / 10;
//   int third = (val % 100) % 10;

//   seq[0] = first;
//   seq[1] = second;
//   seq[2] = third;
// }

/* display the sequence on the terminal window, using the format from the sample run in the spec */
void showSeq(int *seq)
{
  printf("Secret: ");
  for (int i = 0; i < 3; i++)
  {
    printf("%d ", seq[i]);
  }
  printf("\n");
}

#define NAN1 8
#define NAN2 9

/* counts how many entries in seq2 match entries in seq1 */
/* returns exact and approximate matches, either both encoded in one value, */
/* or as a pointer to a pair of values */
int /* or int* */ countMatches(int *seq1, int *seq2)
{
  int exactMatches = 0;
  int approximateMatches = 0;

  int seq1Matched[3] = {0};
  int seq2Matched[3] = {0};

  // Count exact matches
  for (int i = 0; i < 3; i++)
  {
    if (seq1[i] == seq2[i])
    {
      exactMatches++;
      seq1Matched[i] = 1;
      seq2Matched[i] = 1;
    }
  }

  // Count approximate matches
  for (int i = 0; i < 3; i++)
  {
    if (!seq1Matched[i])
    {
      for (int j = 0; j < 3; j++)
      {
        if (!seq2Matched[j] && seq1[i] == seq2[j])
        {
          approximateMatches++;
          seq1Matched[i] = 1;
          seq2Matched[j] = 1;
          break;
        }
      }
    }
  }

  return exactMatches * 10 + approximateMatches;
}

/* show the results from calling countMatches on seq1 and seq1 */
void showMatches(int /* or int* */ code, /* only for debugging */ int *seq1, int *seq2, /* optional, to control layout */ int lcd_format)
{
  /* Assuming the sequences are inputted */
  // int encodedMatches = countMatches(seq1, seq2); // exact in tens, approximate in ones
  // int approx = encodedMatches % 10;
  // int exact = (encodedMatches - approx) / 10;

  /* Assuming code is the output of countMatches */
  int approx = code % 10;
  int exact = (code - approx) / 10;

  printf("Exact matches: %d\nApproximate matches: %d\n", exact, approx);
}

/* parse an integer value as a list of digits, and put them into @seq@ */
/* needed for processing command-line with options -s or -u            */
void readSeq(int *seq, int val)
{

  if (seq == NULL)
  {
    seq = (int *)malloc(LENGTH * sizeof(int));
  }

  int first = val / 100;
  int second = (val % 100) / 10;
  int third = (val % 100) % 10;

  seq[0] = first;
  seq[1] = second;
  seq[2] = third;
}

/* read a guess sequence fron stdin and store the values in arr */
/* only needed for testing the game logic, without button input */
int readNum(int max)
{
  printf("Enter %d numbers.\n", LENGTH);

  int inputs[LENGTH];
  int counter = 0;
  int input;
  while (counter < LENGTH)
  {
    scanf("%d", &input);

    if (input > COLORS || input < 1)
    {
      printf("Input range is between 1 and %d. Terminating\n", COLORS);
      return 1;
    }

    inputs[counter] = input;
    counter++;
  }
  return 0;
}

// The ARM assembler version of the matching fct
extern int /* or int* */ matches(int *val1, int *val2);

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main(int argc, char **argv)
{
  int res, res_c, t, t_c, m, n;
  int *seq1, *seq2, *cpy1, *cpy2;
  struct timeval t1, t2;
  char str_in[20], str[20] = "some text";
  int verbose = 0, debug = 0, help = 0, opt_s = 0, opt_n = 0;

  // see: man 3 getopt for docu and an example of command line parsing
  { // see the CW spec for the intended meaning of these options
    int opt;
    while ((opt = getopt(argc, argv, "hvs:n:")) != -1)
    {
      switch (opt)
      {
      case 'v':
        verbose = 1;
        break;
      case 'h':
        help = 1;
        break;
      case 'd':
        debug = 1;
        break;
      case 's':
        opt_s = atoi(optarg);
        break;
      case 'n':
        opt_n = atoi(optarg);
        break;
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-h] [-v] [-s <seed>] [-n <no. of iterations>]  \n", argv[0]);
        exit(EXIT_FAILURE);
      }
    }
  }

  seq1 = (int *)malloc(seqlen * sizeof(int));
  seq2 = (int *)malloc(seqlen * sizeof(int));
  cpy1 = (int *)malloc(seqlen * sizeof(int));
  cpy2 = (int *)malloc(seqlen * sizeof(int));

  if (argc > optind + 1)
  {
    strcpy(str_in, argv[optind]);
    m = atoi(str_in);
    strcpy(str_in, argv[optind + 1]);
    n = atoi(str_in);
    fprintf(stderr, "Testing matches function with sequences %d and %d\n", m, n);
  }
  else
  {
    int i, j, n = 10, res, res_c, oks = 0, tot = 0; // number of test cases
    fprintf(stderr, "Running tests of matches function with %d pairs of random input sequences ...\n", n);
    if (opt_n != 0)
      n = opt_n;
    if (opt_s != 0)
      srand(opt_s);
    else
      srand(1701);
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < seqlen; j++)
      {
        seq1[j] = (rand() % seqlen + 1);
        seq2[j] = (rand() % seqlen + 1);
      }
      memcpy(cpy1, seq1, seqlen * sizeof(int));
      memcpy(cpy2, seq2, seqlen * sizeof(int));
      if (verbose)
      {
        fprintf(stderr, "Random sequences are:\n");
        showSeq(seq1);
        showSeq(seq2);
      }
      res = matches(seq1, seq2); // extern; code in matches.s
      memcpy(seq1, cpy1, seqlen * sizeof(int));
      memcpy(seq2, cpy2, seqlen * sizeof(int));
      res_c = countMatches(seq1, seq2); // local C function
      if (debug)
      {
        fprintf(stdout, "DBG: sequences after matching:\n");
        showSeq(seq1);
        showSeq(seq2);
      }
      fprintf(stdout, "Matches (encoded) (in C):   %d\n", res_c);
      fprintf(stdout, "Matches (encoded) (in Asm): %d\n", res);
      memcpy(seq1, cpy1, seqlen * sizeof(int));
      memcpy(seq2, cpy2, seqlen * sizeof(int));
      showMatches(res_c, seq1, seq2, 0);
      showMatches(res, seq1, seq2, 0);
      tot++;
      if (res == res_c)
      {
        fprintf(stdout, "__ result OK\n");
        oks++;
      }
      else
      {
        fprintf(stdout, "** result WRONG\n");
      }
    }
    fprintf(stderr, "%d out of %d tests OK\n", oks, tot);
    exit(oks == tot ? 0 : 1);
  }

  readSeq(seq1, m);
  readSeq(seq2, n);

  memcpy(cpy1, seq1, seqlen * sizeof(int));
  memcpy(cpy2, seq2, seqlen * sizeof(int));
  memcpy(seq1, cpy1, seqlen * sizeof(int));
  memcpy(seq2, cpy2, seqlen * sizeof(int));

  gettimeofday(&t1, NULL);
  res_c = countMatches(seq1, seq2); // local C function
  gettimeofday(&t2, NULL);
  // d = difftime(t1,t2);
  if (t2.tv_usec < t1.tv_usec) // Counter wrapped
    t_c = (1000000 + t2.tv_usec) - t1.tv_usec;
  else
    t_c = t2.tv_usec - t1.tv_usec;

  if (debug)
  {
    fprintf(stdout, "DBG: sequences after matching:\n");
    showSeq(seq1);
    showSeq(seq2);
  }
  memcpy(seq1, cpy1, seqlen * sizeof(int));
  memcpy(seq2, cpy2, seqlen * sizeof(int));

  gettimeofday(&t1, NULL);
  res = matches(seq1, seq2); // extern; code in hamming4.s
  gettimeofday(&t2, NULL);
  // d = difftime(t1,t2);
  if (t2.tv_usec < t1.tv_usec) // Counter wrapped
    t = (1000000 + t2.tv_usec) - t1.tv_usec;
  else
    t = t2.tv_usec - t1.tv_usec;

  if (debug)
  {
    fprintf(stdout, "DBG: sequences after matching:\n");
    showSeq(seq1);
    showSeq(seq2);
  }

  memcpy(seq1, cpy1, seqlen * sizeof(int));
  memcpy(seq2, cpy2, seqlen * sizeof(int));
  showMatches(res_c, seq1, seq2, 0);
  showMatches(res, seq1, seq2, 0);

  if (res == res_c)
  {
    fprintf(stdout, "__ result OK\n");
  }
  else
  {
    fprintf(stdout, "** result WRONG\n");
  }
  fprintf(stderr, "C   version:\t\tresult=%d (elapsed time: %dms)\n", res_c, t_c);
  fprintf(stderr, "Asm version:\t\tresult=%d (elapsed time: %dms)\n", res, t);

  return 0;
}