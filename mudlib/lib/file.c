
int file_exists(string filename) {
  return file_size(filename) != -1;
}

int is_directory(string filename) {
  return file_size(filename) == -2;
}

string basename(string filename) {
  return explode(filename, "/")[<1];
}

string dirname(string filename) {
  if (is_directory(filename)) {
    return filename;
  }
  return implode(explode(filename, "/")[0..<2], "/");
}
