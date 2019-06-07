#include "CSVReader.h"

CCSVReader::CCSVReader(std::istream &in) : input(in)
{
    csv_init(&parser, 0);
    //Ptr = this;
}

CCSVReader::~CCSVReader()
{
    csv_free(&parser);
}

void CallbackField(void *str, size_t len, void *data)
{
  CCSVReader *Ptr = static_cast<CCSVReader*>(data);
  std::string buf = std::string((char *)str, len);
  //std::cout << "CallbackField: " << buf << "\n";
  Ptr->rowBuffer.push_back(buf);

}

void CallbackRow(int c, void *data)
{
  CCSVReader *Ptr = static_cast<CCSVReader*>(data);
  //std::cout << "CallbackRow: " << c << "\n";
}



bool CCSVReader::End() const
{
  return input.eof();
}

/**
 * Read row from the input. The parameter row will be filled in with information from the parser
 */
bool CCSVReader::ReadRow(std::vector<std::string> &row)
{

  //read a line from the input file
  std::string buffer;
  if (std::getline(input, buffer)) {

    //std::cout << "Read line: [" << buffer << "]\n";

    //parse the line into rowBuffer (member variable)
    rowBuffer.clear();
    csv_parse(&parser, buffer.c_str(), buffer.length(), CallbackField, CallbackRow, this);
    csv_fini(&parser, CallbackField, CallbackRow, this);

    //copy rowBuffer into the row parameter
    row = rowBuffer;

    return true;

  } else {
    return false;
  }
  // csv_fini(&parser, CallbackField, CallbackRow, this);

}
