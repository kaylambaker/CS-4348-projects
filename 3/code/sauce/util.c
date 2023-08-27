#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../headers/util.h"

bool empty_dir(const char *dirName) {
  char path[256];
  strcpy(path, dirName);
  if (remove(".spool") == 0) { // delete file and make directory
    if (mkdir(dirName, 0777) != 0)
      return false;
    return true;
  }
  DIR *dir;
  struct dirent *file;
  if (!(dir = opendir(dirName)))
    return false;
  while ((file = readdir(dir)) != NULL) {
    if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
      strcat(path, file->d_name);
      if (remove(path) == -1)
        return false;
      memset(path, 0, 256);
      strcpy(path, dirName);
    }
  }
  return true;
}
