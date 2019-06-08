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
  std::cerr<< "Read the first entity. "<<std::endl;
  if ((TempEntity.DType != CXMLEntity::EType::StartElement) or (TempEntity.DNameData != "osm")) {
    return false;
  }

  Node currentNode;
  currentNode.NodeID = InvalidNodeID;
  //when you get to a way node, create a new edge
  //set currentEdge to newEdge
  //need another else if statement line 84


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
        //you have to treat these like a stack
        //

      } else if(TempEntity.DNameData == "nd") {
        //this is gonna be the node that it passes through
        if(currentNode.NodeID == InvalidNodeID) {

          //every time you come to a way tag. set currentNode.NODEID to InvalidNodeID

          //if the curre
          //in here set curretnNode to the the node associated with the reference id
          //AttributeValue of id, look up that node by its id (NodeTranslation) take that node set current node equal to that
        } else {
          //heres the case if its not invalid
          //look up the node associated with this reference
          //then add an edge to the currentNode. using the ID of this ND tag
          //to do that do two things:
          //calculate distance between current Node and the node that you just looked up in this else statement(use HaversineDistance)
          //set edge to the current node, so your gonna have a distance and ID and then you are going to add that to the current NODE
          //last step: set currentNode = to the NODE thats in this else script

          //2 is not invalid -> look up distance between nodeId 1 and 2 and then set the corresponding fields
          //then set currentNode to 2
          //currentNode -> is really the node in the past

        }


      }
    }
  }
  std::sort(SortedNodeIDs.begin(), SortedNodeIDs.end());
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
  return SortedNodeIDs.size();
}







//tonight
CMapRouter::TNodeID CMapRouter::GetNodeIDByStopID(TStopID stopid) const{
    // Your code HERE
  return 0L;
}

std::string CMapRouter::GetSortedRouteNameByIndex(size_t index) const{
  // Your code HERE
  return "";
}

bool CMapRouter::GetRouteStopsByRouteName(const std::string &route, std::vector< TStopID > &stops){
  // Your code HERE
  return true;
}





//do this Friday
double CMapRouter::FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID > &path){
  // Your code HERE
  return 0.0;
}

double CMapRouter::FindFastestPath(TNodeID src, TNodeID dest, std::vector< TPathStep > &path){
  // Your code HERE
  return 0.0;
}

bool CMapRouter::GetPathDescription(const std::vector< TPathStep > &path, std::vector< std::string > &desc) const {
  return true;
}
