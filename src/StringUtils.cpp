#include "StringUtils.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include<string.h>
#include<vector>

namespace StringUtils{

std::string Slice(const std::string &str, ssize_t start, ssize_t end){
  std::string slicedString;
  if(end == 0) {
    end = str.length();
  }

  if( end < 0) {
    end = str.length() - (abs(end));
  }

  if (start < 0) {
    start = str.length() - (abs(start));
  }

  if ( start > str.length()) {
    return "";
  }

  for(int i = start; i < end; i++) {
    slicedString += str[i];
  }

  return slicedString;
}

std::string Capitalize(const std::string &str){
  std::string capString;

  for(int i = 0; i < str.length(); i++) {
    if (isalpha(str[i])) {
      if (i == 0) {
        capString += toupper(str[i]);
      } else {
        capString += tolower(str[i]);
      }
    } else {
      capString += str[i];
    }
  }

  return capString;
}

std::string Title(const std::string &str){
  std::string titleString;

  for(int i = 0; i < str.length(); i++) {
    if (i == 0) {
      titleString += toupper(str[i]);
    }
    if (i != 0) {
      if (!isalpha(str[i - 1])) {
        //if the previous position is not alphabetic
        titleString += toupper(str[i]);
      } else if (isalpha(str[i])) {
        titleString += tolower(str[i]);
      } else {
        titleString += str[i];
      }
    }
  }
  return titleString;
}

std::string LStrip(const std::string &str){
  std::string strippedString;
  bool stop;
  stop = false;
  for(int i = 0; i < str.length(); i++) {
    if ( isspace(str[i]) and !stop) {
      continue;
    } else {
      stop = true;
      strippedString += str[i];
    }
  }
  return strippedString;
}

std::string RStrip(const std::string &str){
  std::string strippedString;
  int numOfSpaces = 0;
  for (int i = str.length(); i >= 0; i--) {
    if ( isspace(str[i]) ) {
      numOfSpaces += 1;
    } else {
      break;
    }
  }
  return str.substr(0, (str.length() - (numOfSpaces + 1)));
}

std::string Strip(const std::string &str){
  std::string strippedString;
  strippedString = LStrip(str);
  strippedString = RStrip(strippedString);
  return strippedString;
}

std::string Center(const std::string &str, int width, char fill){
  std::string adjustedString;
  int sideLengths = (width - str.length());
  bool isOdd = false;

  if (sideLengths % 2 == 1) {
    isOdd = true;
  }

  for (int i = 0; i < (sideLengths / 2); i++) {
    adjustedString += fill;
  }

  for (int x = 0; x < str.length(); x++) {
    adjustedString += str[x];
  }

  if (isOdd) {
    for (int y = 0; y < (sideLengths / 2) + 1; y++) {
      adjustedString += fill;
    }
  }else {
    for (int z = 0; z < (sideLengths / 2); z++) {
      adjustedString += fill;
    }
  }
  return adjustedString;
}

std::string LJust(const std::string &str, int width, char fill){
  std::string adjustedString;
  for (int i = 0; i < width; i++) {
    if(i < str.length() ) {
      adjustedString += str[i];
    } else {
      adjustedString += fill;
    }
  }
  return adjustedString;
}

std::string RJust(const std::string &str, int width, char fill){
  std::string adjustedString;
  for (int i = 0; i < width - str.length(); i++) {
    adjustedString += fill;
  }
  for (int i = 0; i < str.length(); i++) {
    adjustedString += str[i];
  }

  return adjustedString;
}

std::string Replace(const std::string &str, const std::string &old, const std::string &rep){
  std::string newString;
  std::string buffer;
  int placeHolder;
  bool isMatch = false;
  for(int i = 0; i < str.length(); i++){
    buffer = "";
    if (str[i] != old[0]) {
      newString += str[i];
    } else if (str[i] == old[0]) {
      placeHolder = i;
      for(int pos = 0; pos < rep.length() + 1; pos++) {
        if (str[placeHolder] == old[pos]) {
          placeHolder += 1;
          buffer += rep[pos];
        }
        if (buffer.length() == rep.length()) {
          isMatch = true;
        }
      }
      if (isMatch) {
        newString += buffer;
        buffer = "";
        isMatch = false;
        //i = placeHolder;
      } else {
        buffer = "";
        newString += old[i];
      }
    }
  }
  return newString;
}

std::vector< std::string > Split(const std::string &str, const std::string &splt){
  std::vector<std::string> v;
  std::string buffer;
  for(int i = 0; i < str.length(); i++) {
    if (i == str.length() - 1) {
      buffer += str[i];
      v.push_back(buffer);
      break;
    }

    if (str[i] == '/') {
      v.push_back(buffer);
      buffer = "";
    }

    if (str[i] == '/') {
      continue;
    }

    buffer += str[i];
    std::cout << "buffer: " << buffer <<'\n';
  }
  return v;
}

std::string Join(const std::string &str, const std::vector< std::string > &vect){
  std::vector<std::string> v;
  std::string newString;
  int counter = 0;

  //newString = str;
  counter = vect.size();
  for(auto const& value: vect) {
    newString += value;
    counter--;
    if (counter > 0) {
      newString += str;
    }
  }

  return newString;
}

std::string ExpandTabs(const std::string &str, int tabsize){
  std::string newString;
  int buffCounter;
  for (int i = 0; i < str.length(); i++) {
    if (buffCounter == tabsize) {
      buffCounter = 0;
    }
    if (str[i] == '\t') {
      for(int x = buffCounter; x < tabsize; x++) {
        newString += ' ';
      }
    }
    buffCounter += 1;
    newString+= str[i];
  }

  return newString;
}

int EditDistance(const std::string &left, const std::string &right, bool ignorecase){
  int counter = 0;
  int longerString;
  if (left.length() > right.length()) {
    longerString = left.length();
  } else {
    longerString = right.length();
  }

  if (ignorecase) {
    return 0;
  }

  for (int i = 0; i < longerString; i++) {
    if (left[i] == right[i]) {
      continue;
    } else {
      counter += 1;
    }
  }
  return counter;
}

}
