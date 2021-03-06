
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "utils/basic_string_util.h"

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 100;              // max length of vocabulary entries

int load_word2vec_db(char * file_name, char * &vocab, float * vec, float *&M, char ** bestw, long long & words, long long & size){
  // input file location
  // output 
  //     vocab
  //     vec
  //     M
  FILE *f;
  float dist, len; 
  long long a, b; //c, d, cn, bi[100];
  char line[4096] = {0};
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  if (!fgets(line, sizeof(line) - 1, f)) {
    printf("read first line error\n");
    return -2;
  }

  int nn = strlen(line);
  std::vector<std::string> ss;
  BasicStringUtil::SplitString(line, nn, ' ', &ss);
  words = atoi(ss[0].c_str());
  size = atoi(ss[1].c_str());  
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));  
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    if (!fgets(line, sizeof(line) - 1, f)) {
      break;
    }
    std::vector<std::string> comps;
    nn = strlen(line);
    BasicStringUtil::SplitString(line, nn, ' ', &comps);
    memcpy(&vocab[b * max_w], comps[0].c_str(), comps[0].size());
    vocab[b * max_w + comps[0].size()] = 0;
    //printf("%s\n",&vocab[0]);
    char* pEnd = nullptr;
    for (a = 0; a < size; a++) {
      M[a + b * size] = strtof(comps[a + 1].c_str(), &pEnd);
    }
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);
  return 1;
}


int distance_query(char * st1, char ** bestw, float * bestd, char* vocab, float *vec, float *&M, long long words, long long size){
  
  // input:
  //   vocab
  //   vec
  //   M
  // return bestd, bestw
    long long  a, b, c, d, cn, bi[100];
    float dist, len;
    char st[100][max_size];
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    a = 0;
    cn = 0;
    b = 0;
    c = 0;
    
    while (1) {
      st[cn][b] = st1[c];
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0) break;
      if (st1[c] == ' ') {
        cn++;
        b = 0;
        c++;
      }      
    }
    cn++;
    for (a = 0; a < cn; a++) {
      for (b = 0; b < words; b++) {
        //printf("!!%d %s %s\n", words, &vocab[b * max_w], &st[a]);
        if (!strcmp(&vocab[b * max_w], st[a])) break;        
      }
      if (b == words) b = -1;
      bi[a] = b;
      printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;

      }
    }
    
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {
      if (bi[b] == -1) continue;
      for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
    }
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;
    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    for (c = 0; c < words; c++) {
      a = 0;
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      dist = 0;
      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) {
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }
  return 0;
}
 
int main(int argc, char **argv){
  //FILE *f;
  char st1[max_size];
  char *bestw[N];
  char file_name[max_size], st[100][max_size];
  float  bestd[N], vec[max_size];
  long long words, size; //, a, b, c, d, cn, bi[100];
  long long a;
  float *M;
  char *vocab;
  if (argc < 2) {
    printf("Usage: ./distance <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  strcpy(file_name, argv[1]);  
  printf("The input word2vec file is %s\n", file_name);   
  load_word2vec_db(file_name, vocab, vec, M, bestw, words, size);
  printf("%lld, %lld\n", words, size);
  while (1) {   
    printf("Enter word or sentence (EXIT to break): ");
    a = 0;
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (!strcmp(st1, "EXIT")) break; 
    int b = distance_query(st1, bestw, bestd, vocab, vec, M, words, size);    
    if(b==-1) continue;
    printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
    for (a = 0; a < N; a++) printf("%50s\t\t%f\n", bestw[a], bestd[a]);
  }
  
  return 0;
} 




/*
int main(int argc, char **argv) {

  FILE *f;
  char st1[max_size];
  char *bestw[N];
  char file_name[max_size], st[100][max_size];
  float dist, len, bestd[N], vec[max_size];
  long long words, size, a, b, c, d, cn, bi[100];
  float *M;
  char *vocab;
  if (argc < 2) {
    printf("Usage: ./distance <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  strcpy(file_name, argv[1]);
  char line[4096] = {0};

  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  if (!fgets(line, sizeof(line) - 1, f)) {
    printf("read first line error\n");
    return -2;
  }
  int nn = strlen(line);
  std::vector<std::string> ss;
  BasicStringUtil::SplitString(line, nn, ' ', &ss);
  words = atoi(ss[0].c_str());
  size = atoi(ss[1].c_str());
  printf("words:%d, and size:%d\n", words, size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    if (!fgets(line, sizeof(line) - 1, f)) {
      break;
    }
    std::vector<std::string> comps;
    nn = strlen(line);
    BasicStringUtil::SplitString(line, nn, ' ', &comps);
    memcpy(&vocab[b * max_w], comps[0].c_str(), comps[0].size());
    vocab[b * max_w + comps[0].size()] = 0;
    char* pEnd = nullptr;
    for (a = 0; a < size; a++) {
      M[a + b * size] = strtof(comps[a + 1].c_str(), &pEnd);
    }
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);
  while (1) {
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    printf("Enter word or sentence (EXIT to break): ");
    a = 0;
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (!strcmp(st1, "EXIT")) break;
    cn = 0;
    b = 0;
    c = 0;
    while (1) {
      st[cn][b] = st1[c];
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0) break;
      if (st1[c] == ' ') {
        cn++;
        b = 0;
        c++;
      }
    }
    cn++;
    for (a = 0; a < cn; a++) {
      for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
      if (b == words) b = -1;
      bi[a] = b;
      printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
    if (b == -1) continue;
    printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {
      if (bi[b] == -1) continue;
      for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
    }
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;
    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    for (c = 0; c < words; c++) {
      a = 0;
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      dist = 0;
      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) {
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }
    for (a = 0; a < N; a++) printf("%50s\t\t%f\n", bestw[a], bestd[a]);
  }
  return 0;
}
*/
