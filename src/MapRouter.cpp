#include "MapRouter.h"
#include <cmath>
#include "CSVReader.h"
#include "XMLReader.h"
#include "XMLEntity.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <vector>

const CMapRouter::TNodeID CMapRouter::InvalidNodeID = -1;

CMapRouter::CMapRouter(){


}

CMapRouter::~CMapRouter(){

}

double CMapRouter::HaversineDistance(double lat1, double lon1, double lat2, double lon2){
  auto DegreesToRadians = [](double deg){return M_PI * (deg) / 180.0;};
  double LatRad1 = DegreesToRadians(lat1);
  double LatRad2 = DegreesToRadians(lat2);
  double LonRad1 = DegreesToRadians(lon1);
  double LonRad2 = DegreesToRadians(lon2);
  double DeltaLat = LatRad2 - LatRad1;
  double DeltaLon = LonRad2 - LonRad1;
  double DeltaLatSin = sin(DeltaLat/2);
  double DeltaLonSin = sin(DeltaLon/2);
  double Computation = asin(sqrt(DeltaLatSin * DeltaLatSin + cos(LatRad1) * cos(LatRad2) * DeltaLonSin * DeltaLonSin));
  const double EarthRadiusMiles = 3959.88;

  return 2 * EarthRadiusMiles * Computation;
}

double CMapRouter::CalculateBearing(double lat1, double lon1,double lat2, double lon2){
  auto DegreesToRadians = [](double deg){return M_PI * (deg) / 180.0;};
  auto RadiansToDegrees = [](double rad){return 180.0 * (rad) / M_PI;};
  double LatRad1 = DegreesToRadians(lat1);
  double LatRad2 = DegreesToRadians(lat2);
  double LonRad1 = DegreesToRadians(lon1);
  double LonRad2 = DegreesToRadians(lon2);
  double X = cos(LatRad2)*sin(LonRad2-LonRad1);
  double Y = cos(LatRad1)*sin(LatRad2)-sin(LatRad1)*cos(LatRad2)*cos(LonRad2-LonRad1);
  return RadiansToDegrees(atan2(X,Y));
}


bool CMapRouter::LoadMapAndRoutes(std::istream &osm, std::istream &stops, std::istream &routes){
  CXMLReader OSMReader(osm);
  CXMLEntity TempEntity;

  OSMReader.ReadEntity(TempEntity, true);
  if ((TempEntity.DType != CXMLEntity::EType::StartElement) or (TempEntity.DNameData != "osm")) {
    return false;
  }
//used for the way XML tags
  Node prevNode;
  prevNode.NodeID = InvalidNodeID;

  while (!OSMReader.End()){
    OSMReader.ReadEntity(TempEntity, true);
    if(TempEntity.DType == CXMLEntity::EType::StartElement){
      if(TempEntity.DNameData == "node"){
        TNodeID TempNodeID = std::stoul(TempEntity.AttributeValue("id"));
        double TempLat = std::stod(TempEntity.AttributeValue("lat"));
        double TempLon = std::stod(TempEntity.AttributeValue("lon"));
        Node TempNode;
        TempNode.NodeID = TempNodeID;
        TempNode.Location = std::make_pair(TempLat, TempLon);
        NodeTranslation[TempNodeID] = TempNode;
        Nodes.push_back(TempNode);
        SortedNodeIDs.push_back(TempNodeID);
      } else if(TempEntity.DNameData == "way"){
        prevNode.NodeID = InvalidNodeID;
      } else if(TempEntity.DNameData == "nd") {
        if(prevNode.NodeID == InvalidNodeID) {
          //sets node for first node in the way
          TNodeID tempNodeID = std::stoul (TempEntity.AttributeValue("ref"));
          prevNode = NodeTranslation.at(tempNodeID);
        } else {
          //contiunes the train of nodes after the first one
          TNodeID tempNodeID = std::stoul (TempEntity.AttributeValue("ref"));
          Node currNode = NodeTranslation.at(tempNodeID);
          Edge TempEdge;
          //distance between the previous node and the current node
          TempEdge.Distance = HaversineDistance(prevNode.Location.first, prevNode.Location.second,
                                                currNode.Location.first, currNode.Location.second);
          TempEdge.DestNodeID = tempNodeID;

          //add to list of edges
          prevNode.Edges.push_back(TempEdge);
          prevNode = currNode;
        }
      }
    }
  }

  std::sort(SortedNodeIDs.begin(), SortedNodeIDs.end());

  //now loading the routes
  CCSVReader RoutesReader(routes);
  std::vector<std::string> row;
  RoutesReader.ReadRow(row); //need to skip first line because its the header

  while (RoutesReader.ReadRow(row)) {
    if (Routes.find(row[0]) == Routes.end() ) {
      //this means the key does not exist yet and we are putting it into the map
      Routes[row[0]] = std::vector<TStopID>();
      SortedRouteNames.push_back(row[0]);
    }

    Routes.at(row[0]).push_back(std::stoul(row[1]));
  }
  std::sort(SortedRouteNames.begin(), SortedRouteNames.end());

  //now loading the stops
  CCSVReader StopsReader(stops);
  StopsReader.ReadRow(row); //need to skip first line because its the header

  while (StopsReader.ReadRow(row)) {
    //Stops
    TStopID stopID = std::stoul(row[0]);
    TNodeID nodeID = std::stoul(row[1]);
    Stops[stopID] = nodeID;
    SortedStopIDs.push_back(stopID);
  }
  std::sort(SortedStopIDs.begin(), SortedStopIDs.end());
  return true;


}

size_t CMapRouter::NodeCount() const{
  return SortedNodeIDs.size();
}

/**
 * Use the given index to get the ID of the node within SortedNodeIDs
 */
CMapRouter::TNodeID CMapRouter::GetSortedNodeIDByIndex(size_t index) const{

    //return std::make_pair(180.0, 360.0);
  if ( index >= SortedNodeIDs.size()) {
      return InvalidNodeID;
  }

  return SortedNodeIDs[index];
}


/**
 * Get the location of the node by node ID
 */
CMapRouter::TLocation CMapRouter::GetNodeLocationByID (TNodeID nodeID) const{
if (nodeID == InvalidNodeID) {
  return std::make_pair(180.0, 360.0);
}
  //Find the key and value (Search) of the nodeID within the map
  //the key (the node ID) will be Search->first
  //and the value (the node) will be Search->second

  auto Search = NodeTranslation.find(nodeID);



  //if given ID not in the dataset



  //get the location of the node that was found
  return (Search->second).Location;



}

/**
 * Get the location of the node by index within the sorted nodes
 */

CMapRouter::TLocation CMapRouter::GetSortedNodeLocationByIndex(size_t index) const{


  return GetNodeLocationByID(GetSortedNodeIDByIndex(index));
}

size_t CMapRouter::RouteCount() const{
  return Routes.size();
}

//tonight
CMapRouter::TNodeID CMapRouter::GetNodeIDByStopID(TStopID stopid) const{

  return Stops.at(stopid);
}

std::string CMapRouter::GetSortedRouteNameByIndex(size_t index) const{
  return SortedRouteNames[index];

}

bool CMapRouter::GetRouteStopsByRouteName(const std::string &route, std::vector< TStopID > &stops){
  if (Routes.find(route) != Routes.end()) {
    stops = Routes.at(route);
    return true;
  }
  return false;

}

//do this Friday
double CMapRouter::FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID > &path){

  //node id -> shortest distance to every node from the initial source node
  std::map<TNodeID, double> distances;
  for (int i = 0; i < SortedNodeIDs.size(); i++) {
    distances[SortedNodeIDs[i]] = std::numeric_limits<double>::infinity();
  }

  //node id -> shortest paths from the initial source node
  std::map<TNodeID, std::vector<TNodeID>> paths;
  for (int i = 0; i < SortedNodeIDs.size(); i++) {
    paths[SortedNodeIDs[i]] = std::vector<TNodeID>();
  }

  //list of unevaluated nodes IDs
  std::set<TNodeID> unevaluated;
  std::set<TNodeID> evaluated;

  //initialize the first distance and node
  distances[src] = 0.0;
  unevaluated.insert(src);
  paths[src].push_back(src);

  //find the shortest distance and path to each node in the graph from the source node
  while (unevaluated.size() > 0) {

    //Find the node ID with the minimum distance in the uneval set
    TNodeID minNodeID = InvalidNodeID;
    double minDistance = std::numeric_limits<double>::infinity();
    for (std::set<TNodeID>::iterator iter = unevaluated.begin(); iter != unevaluated.end(); ++iter) {
      TNodeID nodeID = *iter;
      double distance = distances[nodeID];
      if (distance < minDistance) {
	       minDistance = distance;
	        minNodeID = nodeID;
      }
    }

    unevaluated.erase(minNodeID);

    for (int i = 0; i < NodeTranslation.at(minNodeID).Edges.size(); i++) {
      TNodeID edgeNodeID = NodeTranslation.at(minNodeID).Edges[i].DestNodeID;
      double edgeDistance = NodeTranslation.at(minNodeID).Edges[i].Distance;
      if (!evaluated.count(edgeNodeID)) {
	       double newDistance = distances[minNodeID] + edgeDistance;

	       //if new distance is less than the stored instance
	       //reset the distance and path to the new min
	        if (newDistance < distances[edgeNodeID]) {
	           distances[edgeNodeID] = newDistance;
	           paths[edgeNodeID] = std::vector<TNodeID>();
	           for (int j = 0; j < paths[minNodeID].size(); j++) {
	              paths[edgeNodeID].push_back(paths[minNodeID][j]);
	           }
	           paths[edgeNodeID].push_back(edgeNodeID);
	         }
	          unevaluated.insert(edgeNodeID);
      }

    }

    evaluated.insert(minNodeID);

  } //end of the while loop


  //set path to the minimum path to the dest node
  path = paths[dest];

  //return the minimum distance to the dest node
  return distances[dest];
    
}


double CMapRouter::FindFastestPath(TNodeID src, TNodeID dest, std::vector< TPathStep > &path){
  // Your code HERE
  return 0.0;
}

bool CMapRouter::GetPathDescription(const std::vector< TPathStep > &path, std::vector< std::string > &desc) const {
  return true;
}
