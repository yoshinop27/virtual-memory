#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  // Added: seed rand() so messages vary each run
#include <unistd.h>

const char* get_msg(const char* msg[]);
void seg_handler(int signal, siginfo_t* info, void* ctx);

const char* messages[] = {"Encouraging message 1", "Encouraging message 2", "Encouraging message 3",
                          "Encouraging message 4", "Encouraging message 5", "Encouraging message 6",
                          "Encouraging message 7", "Encouraging message 8"};

// TODO: Implement your signal handling code here!
__attribute__((constructor)) void init() {
  srand((unsigned)(time(NULL) ^ getpid()));  // Cite: man 3 srand, time
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = seg_handler;
  sa.sa_flags = SA_SIGINFO;  // Cite: man 2 sigaction

  if (sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }
}

// function handler for segmentation faults
void seg_handler(int signal, siginfo_t* info, void* ctx) {
  printf("%s", get_msg(messages));
  // Why: after a SIGSEGV we must end the program;
  _exit(1);
}

// generate random message
const char* get_msg(const char* msg[]) {
  int idx = rand() % 8;
  return msg[idx];
}
