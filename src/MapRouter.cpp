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


  //while (!OSMReader.End()){
  while(OSMReader.ReadEntity(TempEntity)) {
    OSMReader.ReadEntity(TempEntity, true);
    if(TempEntity.DType == CXMLEntity::EType::StartElement){
      std::cerr<< "-------FOUND START------"<<std::endl;
      if(TempEntity.DNameData == "node"){
        std::cerr<< "-------FOUND NODE------"<<std::endl;
        TNodeID TempNodeID = std::stoul(TempEntity.AttributeValue("id"));
        double TempLat = std::stod(TempEntity.AttributeValue("lat"));
        double TempLon = std::stod(TempEntity.AttributeValue("lon"));
        Node TempNode;
        TempNode.NodeID = TempNodeID;
        TempNode.Location = std::make_pair(TempLat, TempLon);
        //NodeTranslation[Node.size()] = TempNodeID;  was Node.size() is This adding an item to the map?
        NodeTranslation[TempNodeID] = TempNode;
        Nodes.push_back(TempNode);
        SortedNodeIDs.push_back(TempNodeID);
      } else if(TempEntity.DNameData == "way"){
        std::cerr<< "------FOUND WAY-------"<<std::endl;
      }
    }
  }
  OSMReader.End();
  std::sort(SortedNodeIDs.begin(), SortedNodeIDs.end());
  return true;

}

size_t CMapRouter::NodeCount() const{
  std::cerr<< "Code enters Node Count())"<<std::endl;
  std::cerr<< "size of sorted node ids: " << SortedNodeIDs.size()  <<std::endl;
  std::cerr<< "size of NODES: " << Nodes.size()  <<std::endl;
  return SortedNodeIDs.size();
}

/**
 * Use the given index to get the ID of the node within SortedNodeIDs
 */
CMapRouter::TNodeID CMapRouter::GetSortedNodeIDByIndex(size_t index) const{
  std::cerr<< "Code enters by id()"<<std::endl;

  return SortedNodeIDs[index];
}


/**
 * Get the location of the node by node ID
 */
CMapRouter::TLocation CMapRouter::GetNodeLocationByID (TNodeID nodeID) const{

  //Find the key and value (Search) of the nodeID within the map
  //the key (the node ID) will be Search->first
  //and the value (the node) will be Search->second
  auto Search = NodeTranslation.find(nodeID);

  //get the location of the node that was found
  return (Search->second).Location;

}

/**
 * Get the location of the node by index within the sorted nodes
 */

CMapRouter::TLocation CMapRouter::GetSortedNodeLocationByIndex(size_t index) const{
  std::cerr<< "Code enters by location()"<<std::endl;

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
