#include "XMLReader.h"

void StartElementHandler (void *data, const XML_Char *name, const XML_Char **atts) {
  CXMLReader *ptr = static_cast<CXMLReader*>(data);
  //std::cout << "StartElementHandler: " << name << "\n";

  SXMLEntity entity;

  //not sure how to get the type here
  //courseEntity.DType = SXMLEntity::EType::StartElement;
  entity.DNameData = name;

  //attributes come as an flattened array - need to pair things up
  //TODO: error checking around the pointers
  while (*atts != NULL) {
    //std::cout << "StartElementHandler: attr " << *atts << "\n";
    std::string attrName = *atts;
    atts++;
    std::string attrValue = *atts;
    atts++;
    entity.SetAttribute(attrName, attrValue);
  }
  ptr->entities.push_back(entity);
  
}


void EndElementHandler(void *data, const XML_Char *name) {
  //std::cout << "EndElementHandler: " << name << "\n";
}

CXMLReader::CXMLReader(std::istream &in) : input(in) {

  
  //this is a DOM parser so parse the whole thing in the constructor
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, StartElementHandler, EndElementHandler);
  
  std::string buffer;
  while (std::getline(input, buffer)) {
    //std::cout << "Constructor: read line [" << buffer << "]\n";
    if (XML_Parse(parser, buffer.c_str(), buffer.length(), XML_FALSE) == XML_STATUS_ERROR) {
      std::cerr << "Could not read file\n";
    }
  }
  //std::cout << "Constructor: read " << entities.size() << " entities\n";
  XML_ParserFree(parser);
}

CXMLReader::~CXMLReader()
{
    
}

bool CXMLReader::End()
{
    return input.eof();
}

bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata) {
  if (entityIdx < entities.size()) {
    entity = entities[entityIdx];
    entityIdx++;
    return true;
  } else {
    return false;
  }
  

}
