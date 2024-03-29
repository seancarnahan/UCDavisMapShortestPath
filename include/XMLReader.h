
#ifndef XMLREADER_H
#define XMLREADER_H

#include "XMLEntity.h"
#include <istream>
#include <iostream>
#include <expat.h>
#include <vector>


class CXMLReader{
 private:
  std::istream &input;
  
 public:
  CXMLReader(std::istream &is);
  ~CXMLReader();
  std::vector<CXMLEntity> entities;
  int entityIdx = 0;
  
  bool End();
  bool ReadEntity(CXMLEntity &entity, bool skipcdata = false);
};

#endif
