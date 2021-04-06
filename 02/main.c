#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./pkg/parser/parser.h"
#include "./pkg/strings/strings.h"

#include <sys/stat.h>
#include <fcntl.h>

void runCmd(const Cmd* cmds, int n) {
  if (n < 1) {
    return;
  }
  if (strcmp(cmds->command, "cd") == 0) {
    int err = chdir(cmds->argv[1]);
    if (err == -1) {
      perror("cd");
      exit(EXIT_FAILURE);
    }
    return;
  }
  if (strcmp(cmds[n-1].command, "&") == 0) {
    pid_t pid = fork();
    if (pid == 0) {
      runCmd(cmds, n-1);
      exit(0);
    }
    return;
  }


  int status = 0;
  int in = STDIN_FILENO;
  int i = 0;
  for (; i < n; ++i) {
    if (i > 1 && strcmp(cmds[i-1].command, "||") == 0) {
      pid_t pid = fork();
      if (pid == 0) {
        if (strcmp(cmds[i-1].command, "|") == 0) {
          dup2(in, STDIN_FILENO);
        }
        execvp(cmds[i].command, cmds[i].argv); 
      }
      if (WEXITSTATUS(status) == 1) {
        waitpid(pid, &status, 0);
      }
      continue;
    }
    if(WEXITSTATUS(status) == 1 && strcmp(cmds[i].command, "||") != 0) {
      return;
    };
    if (i + 1 < n && strcmp(cmds[i+1].command, "|") == 0) {
      int fd[2];
      int err = pipe(fd);
      if (err == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
      }
      pid_t pid = fork();
      if (pid == 0) {
        if (i > 1 && strcmp(cmds[i-1].command, "|") == 0) {
          dup2(in, STDIN_FILENO);
        }
        dup2(fd[1], STDOUT_FILENO);
        err = execvp(cmds[i].command, cmds[i].argv);
        if (err == -1) {
          exit(EXIT_FAILURE);
        }
      }
      close(fd[1]);
      waitpid(pid, &status, 0);
      in = fd[0];
      ++i;
    } else if (i + 2 < n && strcmp(cmds[i+1].command, ">>") == 0) {
      int fd[2];
      int err = pipe(fd);
      if (err == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
      }
      pid_t pid = fork();
      if (pid == 0) {
        if (i > 1 && strcmp(cmds[i-1].command, "|") == 0) {
          dup2(in, STDIN_FILENO);
        }
        dup2(fd[1], STDOUT_FILENO);
        err = execvp(cmds[i].command, cmds[i].argv);
        if (err == -1) {
          printf("error\n");
          exit(EXIT_FAILURE);
        }
      } 
      close(fd[1]);
      wait(NULL);
      int filedes = open(cmds[i + 2].command, O_RDWR | O_CREAT | O_APPEND, 
              S_IWUSR | S_IRUSR);
      size_t size;
      char buf[1024];
      while((size=read(fd[0], buf, 1024)) > 0) {
        err = write(filedes, buf, size);
        if (err == -1) {
          exit(EXIT_FAILURE);
        }
      }
      close(filedes);
      i += 2;
    } else if (i + 2 < n && strcmp(cmds[i+1].command, ">") == 0) {
      int fd[2];
      int err = pipe(fd);
      if (err == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
      }
      pid_t pid = fork();
      if (pid == 0) {
        if (i > 1 && strcmp(cmds[i-1].command, "|") == 0) {
          dup2(in, STDIN_FILENO);
        }
        dup2(fd[1], STDOUT_FILENO);
        execvp(cmds[i].command, cmds[i].argv);
      } 
      close(fd[1]);
      wait(NULL);
      int filedes = open(cmds[i + 2].command, O_RDWR | O_CREAT | O_TRUNC, 
              S_IWUSR | S_IRUSR);
      size_t size;
      char buf[1024];
      while((size=read(fd[0], buf, 1024)) > 0) {
        err = write(filedes, buf, size);
        if (err == -1) {
          exit(EXIT_FAILURE);
        }
      }
      close(filedes);
      i += 2;
    } else if (strcmp(cmds[i].command, "&&") == 0 || 
        strcmp(cmds[i].command, "||") == 0) {
      continue;
    } else {
      pid_t pid = fork();
      if (pid == 0) {
        if (i > 1 && strcmp(cmds[i-1].command, "|") == 0) {
          dup2(in, STDIN_FILENO);
        }
        execvp(cmds[i].command, cmds[i].argv); 
      }
      waitpid(pid, &status, 0);
    }
  }

  if (in != STDIN_FILENO) {
    close(in);
  }
  while (waitpid(-1, &status, WNOHANG) > 0) {}
}

int main() {
  while(true) { 
    Cmd* cmds = NULL;
    int n = getCmds(&cmds, stdin);
    if (n == -1) {
      return 0;
    }
    runCmd(cmds, n);
    cmdFree(cmds, n);
  }
  return 0;
}
