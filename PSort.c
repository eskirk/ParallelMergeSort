#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define ARR_SIZE 100
#define BIG 99999

void setupChildren(int *left, int *right, int size, int *fdsL, int *fdsR, int *parent);
void splitArr(int *arr, int size, int *fds);

void splitArr(int *arr, int size, int *parent) {
   int *left, *right, half, fdsL[2], fdsR[2];

   if (size >= 2) {
      half = size / 2;
      left = arr;
      right = arr + half;

      pipe(fdsL);
      pipe(fdsR);
      setupChildren(left, right, size, fdsL, fdsR, parent);
      return;
   }
   write(parent[1], arr, sizeof(int));
   return;
}

void setupChildren(int *left, int *right, int size, int *fdsL, int *fdsR, int *parent) {
   int pidL = 0, pidR = 0, count, bits = 0;
   int lhalf, rhalf, *temp = calloc(size, sizeof(int));

   if (size < 2)
      return;

   lhalf = (size / 2);
   rhalf = size - lhalf;

   pidL = fork();
   if (pidL > 0) {
      close(fdsL[1]);
      pidR = fork();
      if (pidR > 0){
         close(fdsR[1]);      
      }
      else if (pidR == 0) {
         close(fdsR[0]);
         close(fdsL[0]);
         close(fdsL[1]);
         splitArr(right, rhalf, fdsR);
         return;
      } else 
         printf("Failure to fork\n");
   }
   else if (pidL == 0) {
      close(fdsL[0]);
      close(fdsR[0]);
      close(fdsR[1]);
      splitArr(left, lhalf, fdsL);
      return;
   } else 
      printf("Failure to fork\n");

   waitpid(pidL, NULL, 0);
   waitpid(pidR, NULL, 0);
   count = 0;
   read(fdsR[0], &pidR, sizeof(int));
   read(fdsL[0], &pidL, sizeof(int));
   while (count < size) {
      if (pidL <= pidR) {
         temp[count++] = pidL;
         bits = read(fdsL[0], &pidL, sizeof(int));
         if (!bits) 
            pidL = INT_MAX;
      }
      else if (pidR < pidL) {
         temp[count++] = pidR;
         bits = read(fdsR[0], &pidR, sizeof(int));
         if (!bits) 
            pidR = INT_MAX;
      }
   }
   write(parent[1], temp, sizeof(int) * size);
}

int main(int argc, char *argv[]) {
   char *scan = malloc(sizeof(char) * 10);
   int *arr = calloc(sizeof(int), ARR_SIZE), *rtn, fds[2];
   int size = 0, count = 0, pid = 0;
   time_t t;

   if (argc == 1) {
      printf("Would you like to enter your own array?\ny/n: ");
      scanf("%c", scan);
      if (strcmp(scan, "y") == 0) {
         printf("Please enter up to 100 integer numbers :^)\n");
         while (strcmp(scan, "q") != 0 && size < ARR_SIZE) {
            scanf("%s", scan);
            arr[size++] = atoi(scan);
         }
      }
      else {
         srand((unsigned) time(&t));
         printf("How large do you want your array?\nEnter an integer less than 100: ");
         scanf("%d", &size);
         while (count < size) {
            arr[count++] = (rand() % 50);
         }
      }
   }
   else {
      size = atoi(argv[1]);
      srand((unsigned) time(&t));
      while (count < size) {
         arr[count++] = (rand() % 50);
      }
   }      
   arr[count] = '\0';

   count = 0;
   printf("OG ARRAY\n");
   while (count < size) {
      printf("%d: %d\n", count, arr[count]);
      count++;
   }
   printf("\n");

   rtn = calloc(count, sizeof(int));
   pipe(fds);
   pid = fork();
   if (pid == 0) {
      close(fds[0]);
      splitArr(arr, size, fds);
   }
   else if (pid > 0) {
      close(fds[1]);
      count = 0;
      waitpid(pid, NULL, 0);
      read(fds[0], rtn, sizeof(int) * size);

      printf("SORTED ARRAY\n");
      do {
         printf("%d: %d\n", count, rtn[count]);
      } while (count++ < size - 1);
   }

   return 0;
}
