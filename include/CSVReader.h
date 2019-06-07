#ifndef CSVREADER_H
#define CSVREADER_H

#include <iostream>
#include <istream>
#include <string>
#include <vector>
#include <csv.h>

class CCSVReader{
 protected:
  struct csv_parser parser;
  std::istream &input;
  
 public:
  CCSVReader(std::istream &in);
  ~CCSVReader();
  std::vector<std::string> rowBuffer;
  
  //void CallbackField(void *str, size_t len, void *data);
  //void CallbackRow(int c, void *data);
  bool End() const;
  bool ReadRow(std::vector< std::string > &row);
};

#endif
