#include "MapRouter.h"
#include <cmath>
#include "CSVReader.h"
#include "XMLReader.h"
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

  OSMReader ReadEntity(TempEntity, true);
  if(TempEntity.DType != SXMLEntity::EType::StartElement) or (TempEntity.DName != "osm"){
    return false;
  }

  while (!OSMReader.End()){
    OSMReader.ReadEntity(TempEntity, true);
    if(TempEntity.DType == SXMLEntity::EType::StartElement){
      if(TempEntity.DName == "node"){
        TNodeID TempNodeID = std::stoul(TempEntity.AttributValue("id"));
        double TempLat = std::stod(TempEntity.AttributValue("lat"));
        double TempLon = std::stod(TempEntity.AttributValue("lon"));
        Node TempNode;
        TempNode.NodeID = TempNodeID;
        TempNode.Location = std::make_pair(TempLat, TempLon);
        NodeTranslation[Node.size()] = TempNodeID;
        Nodes.push_back(TempNode);
        SortedNodeIDs.push_back(TempNodeID)
      } else if(TempEntity.DName == "way"){

      }
    }
  }

  std::sort(SortedNodeIDs.begin(), SortedNodeIDs.end());
  struct Edge{
    double Distance;
    TNodeIndex DestNode;
  };

  struct Node{
    TNodeID NodeID;
    std::std::vector<Edge> Edges;
  };

  std::vector<Node> Nodes;
  std::std::vector<TNodeID> SortedNodeIDs;
}

size_t CMapRouter::NodeCount() const{
  return SortedNodeIds.size();
}

CMapRouter::TNodeID CMapRouter::GetSortedNodeIDByIndex(size_t index) const{
  return SortedNodeIDs[index];
}

CMapRouter::TLocation CMapRouter::GetSortedNodeLocationByIndex(size_t index) const{
  auto Search = NodeTranslation.find(SortedNodeIDs[index]);
  return Nodes[Search -> second].Location;
}

CMapRouter::TLocation CMapRouter::GetNodeLocationByID(TNodeID nodeid) const{
    return Nodes[NodeTranslation.find(nodeid) -> second].Location;
}

CMapRouter::TNodeID CMapRouter::GetNodeIDByStopID(TStopID stopid) const{
    // Your code HERE
}

size_t CMapRouter::RouteCount() const{
  return SortedNodeIds.size();
}

std::string CMapRouter::GetSortedRouteNameByIndex(size_t index) const{
    // Your code HERE
}

bool CMapRouter::GetRouteStopsByRouteName(const std::string &route, std::vector< TStopID > &stops){
    // Your code HERE
}

double CMapRouter::FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID > &path){
    // Your code HERE
}

double CMapRouter::FindFastestPath(TNodeID src, TNodeID dest, std::vector< TPathStep > &path){
    // Your code HERE
}

bool CMapRouter::GetPathDescription(const std::vector< TPathStep > &path, std::vector< std::string > &desc) const;
