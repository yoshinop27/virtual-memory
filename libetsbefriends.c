#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

const char* get_msg(const char* msg[]);
void seg_handler(int signal, siginfo_t* info, void* ctx);
const char *messages[] = {
  "Encouraging message 1",
  "Encouraging message 2",
  "Encouraging message 3",
  "Encouraging message 4",
  "Encouraging message 5",
  "Encouraging message 6",
  "Encouraging message 7",
  "Encouraging message 8"
};

// TODO: Implement your signal handling code here!
__attribute__((constructor)) void init() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = seg_handler;
  sa.sa_flags = SA_SIGINFO;
  if(sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }
}

// function handler segmentation faults
void seg_handler(int signal, siginfo_t* info, void* ctx){
  printf("%s", get_msg(messages));
}

// generate random message
const char* get_msg(const char* msg[]) {
  int idx = rand() % 8;
  return msg[idx];
}

