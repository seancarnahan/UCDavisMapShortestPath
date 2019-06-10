#include "MapRouter.h"
#include "StringUtils.h"
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>

bool isNumber(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

int main(int argc, char** argv) {
  CMapRouter router;
  std::ifstream map("./data/davis.osm");
  std::ifstream stops("./data/stops.csv");
  std::ifstream routes("./data/routes.csv");
  std::ofstream outputFile;
  std::vector<CMapRouter::TPathStep> path;
  std::vector<CMapRouter::TNodeID> nodes;
  router.LoadMapAndRoutes(map, stops, routes);
  double totalDistance, totalTime;
  
  std::cout << "Loaded " << router.NodeCount() << " nodes" << std::endl;
  while (true) {
    std::cout << ">";
    std::string userInput;
    getline(std::cin, userInput);
    if (userInput.length() == 0) {
      continue;
    }
    std::vector <std::string> userInputVector = StringUtils::Split(userInput);
    
    if(userInputVector[0] == "help"){
      std::cout << "findroute [--data=path | --results=path]" << std::endl;
      std::cout << "------------------------------------------------------------------------" << std::endl;
      std::cout << "help     Display this help menu" << std::endl;
      std::cout << "exit     Exit the program" << std::endl;
      std::cout << "count    Output the number of nodes in the map" << std::endl;
      std::cout << "node     Syntax \"node [0, count)" << std::endl;
      std::cout << "         Will output node ID and Lat/Lon for node" << std::endl;
      std::cout << "fastest  Syntax \"fastest start end" << std::endl;
      std::cout << "         Calculates the time for fastest path from start to end" << std::endl;
      std::cout << "shortest Syntax \"shortest start end" << std::endl;
      std::cout << "         Calculates the distance for the shortest path from start to end" << std::endl;
      std::cout << "save     Saves the last calculated path to file" << std::endl;
      std::cout << "print    Prints the steps for the last calculated path" << std::endl;
    }
    
    else if(userInputVector[0] == "exit"){
      break;
    }
  
    else if(userInputVector[0] == "count"){
      std::cout << router.NodeCount() << " nodes" << std::endl;
    }
    
    else if(userInputVector[0] == "node"){
      if(userInputVector.size() == 1){
        std::cout << "Invalid node command, see help." << std::endl;
      } else if (!isNumber(userInputVector[1])/* or (std::stoi(userInputVector[1]) > 10258)*/){
        std::cout << "Invalid node parameter, see help." << std::endl;
      } else{
        std::cout << "Node " << userInputVector[1] << ": id = ";
        std::cout << router.GetSortedNodeIDByIndex(std::stoi(userInputVector[1]));
	CMapRouter::TLocation location = router.GetSortedNodeLocationByIndex(std::stoi(userInputVector[1]));
        std::cout << " is at " << location.first << "," << location.second << std::endl;
      }
    }
    
    else if(userInputVector[0] == "fastest"){
      if(userInputVector.size() < 2){
        std::cout << "Invalid fastest command, see help." << std::endl;
      } else if(!(isNumber(userInputVector[1]) and isNumber(userInputVector[2]))){
        std::cout << "Invalid fastest parameter, see help." << std::endl;
      } else{
        if((std::stoi(userInputVector[1]) < 62208369) or (std::stoi(userInputVector[2]) < 62208369)){
          std::cout << "Unable to find fastest path from " << userInputVector[1] << " to ";
          std::cout << userInputVector[2] << std::endl;
        } else {
          totalTime = router.FindFastestPath(std::stoi(userInputVector[1]), std::stoi(userInputVector[2]), path);
          std::cout << "Fastest path takes " << totalDistance << std::endl;
        }
      }
    }
    
    else if(userInputVector[0] == "shortest"){
      if(userInputVector.size() < 2){
        std::cout << "Invalid shortest command, see help." << std::endl;
      } else if(!(isNumber(userInputVector[1]) and isNumber(userInputVector[2]))){
        std::cout << "Invalid shortest parameter, see help." << std::endl;
      } else{
        if((std::stoi(userInputVector[1]) < 62208369) or (std::stoi(userInputVector[2]) < 62208369)){
          std::cout << "Unable to find shortest path from " << userInputVector[1] << " to ";
          std::cout << userInputVector[2] << std::endl;
        } else {
          totalDistance = router.FindShortestPath(std::stoi(userInputVector[1]), std::stoi(userInputVector[2]), nodes);
          std::cout << "Shortest path is " << totalDistance << "mi" << std::endl;
        }
      }
    }
  
    else if(userInputVector[0] == "save"){
      if(path.size() == 0){
        std::cout << "No valid path to save." << std::endl;
      } else {
        std::vector<std::string> description;
        router.GetPathDescription(path, description);
        outputFile.open("results.txt");
	for (int dIndex = 0; dIndex < description.size(); dIndex++) {
	  outputFile << description[dIndex] << std::endl;
	}
	outputFile.close();
      }
    }
  
    else if (userInputVector[0] == "print"){
      if(path.size() == 0){
	std::cout << "No valid path to print." << std::endl;
      } else {
        std::vector<std::string> description;
        router.GetPathDescription(path, description);
        for(auto direction : description){
          std::cout << direction << std::endl;
        }
      }
    }
    
    else{
      std::cout << "Unknown command \"" << userInputVector[0] << "\" type help for help." << std::endl;
    }
  }
  
  return 1;
}
