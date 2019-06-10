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
#include <set>
#include <cstdio>
#include <cmath>

const CMapRouter::TNodeID CMapRouter::InvalidNodeID = -1;
const std::vector<std::string> BEARINGS{ "N", "NE", "E", "SE", "S", "SW", "W", "NW", "N" };

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
  TNodeID prevNodeID = InvalidNodeID;

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
        prevNodeID = InvalidNodeID;
      } else if(TempEntity.DNameData == "nd") {
        if(prevNodeID == InvalidNodeID) {
          //sets node for first node in the way
          prevNodeID = std::stoul(TempEntity.AttributeValue("ref"));
        } else {
          //contiunes the train of nodes after the first one
          TNodeID currNodeID = std::stoul (TempEntity.AttributeValue("ref"));
	  TLocation currNodeLocation = GetNodeLocationByID(currNodeID);
	  TLocation prevNodeLocation = GetNodeLocationByID(prevNodeID);
	    
          //Node currNode = NodeTranslation.at(tempNodeID);
          //distance between the previous node and the current node
          Edge TempEdge;
          TempEdge.Distance = HaversineDistance(prevNodeLocation.first, prevNodeLocation.second,
                                                currNodeLocation.first, currNodeLocation.second);
          TempEdge.DestNodeID = currNodeID;
	  	  
          //add to list of edges
          NodeTranslation[prevNodeID].Edges.push_back(TempEdge);
          prevNodeID = currNodeID;
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
    if (RouteStopIDs.find(row[0]) == RouteStopIDs.end() ) {
      //this means the key does not exist yet and we are putting it into the map
      SortedRouteNames.push_back(row[0]);
      RouteStopIDs[row[0]] = std::vector<TStopID>();
      RouteNodeIDs[row[0]] = std::vector<TNodeID>();
    }    
    RouteStopIDs.at(row[0]).push_back(std::stoul(row[1]));
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

  //fill in any gaps in nodes for the routes
  for (auto i = 0; i < RouteCount(); i++) {
    std::string routeName = GetSortedRouteNameByIndex(i);
    std::vector<TStopID> routeStopIDs;
    if (GetRouteStopsByRouteName(routeName, routeStopIDs)) {

      //need to sort the route stops for this to work
      std::sort(routeStopIDs.begin(), routeStopIDs.end());
      
      //loop through both vectors simultaneously
      //and fillin any gaps between the stops and the routes
      //TODO: Need to fix to handle when route has a stop not in the
      //      ordered list - hopefully this is not common
      auto sortedStopIDsIter = SortedStopIDs.begin();
      auto routeStopIDsIter = routeStopIDs.begin();
      bool prevMatch = false;
      while (sortedStopIDsIter < SortedStopIDs.end() &&
      	     routeStopIDsIter < routeStopIDs.end()) {	
      	TStopID sortedStopID = *sortedStopIDsIter;
      	TStopID routeStopID  = *routeStopIDsIter;
      	TNodeID sortedNodeID = GetNodeIDByStopID(sortedStopID);
      	if (routeStopID == sortedStopID) {
	  //if there is a match add the node
      	  RouteNodeIDs.at(routeName).push_back(sortedNodeID);
      	  sortedStopIDsIter++;
      	  routeStopIDsIter++;
      	  prevMatch = true;
      	} else if (prevMatch) {
	  //if no match but the previous stop matched then add the node
      	  RouteNodeIDs.at(routeName).push_back(sortedNodeID);
      	  sortedStopIDsIter++;	  
      	} else {
	  sortedStopIDsIter++;
	}
      }
      
    }

  }
  
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
  return RouteStopIDs.size();
}

CMapRouter::TNodeID CMapRouter::GetNodeIDByStopID(TStopID stopid) const{
  return Stops.at(stopid);
}

std::string CMapRouter::GetSortedRouteNameByIndex(size_t index) const{
  return SortedRouteNames[index];
}

bool CMapRouter::GetRouteStopsByRouteName(const std::string &route, std::vector< TStopID > &stops){
  if (RouteStopIDs.find(route) != RouteStopIDs.end()) {
    stops = RouteStopIDs.at(route);
    return true;
  }
  return false;
}

bool CMapRouter::GetRouteNodesByRouteName(const std::string &route, std::vector< TNodeID > &nodes){
  if (RouteNodeIDs.find(route) != RouteNodeIDs.end()) {
    nodes = RouteNodeIDs.at(route);
    return true;
  }
  return false;
}


size_t CMapRouter::GetEdgeCountByID(TNodeID nodeid) const {
  if (NodeTranslation.count(nodeid) > 0) { 
    return NodeTranslation.at(nodeid).Edges.size();
  }
  return 0;
}

CMapRouter::TNodeID CMapRouter::GetEdgeNodeByIndex(TNodeID nodeid, size_t index) const {
  return NodeTranslation.at(nodeid).Edges[index].DestNodeID;
}

double CMapRouter::GetEdgeDistanceByIndex(TNodeID nodeid, size_t index) const {
  return NodeTranslation.at(nodeid).Edges[index].Distance;
}



/**
 * Find minimum path - generic method for both shortest distance and fastest path.
 * This method is just Dijkstra's algorithm for shortest path but takes a function
 * pointer to delegate getting the weight of any particular edge. The edge can 
 * represent either distance or time
 */
double findMinimumPath (CMapRouter::TNodeID src, CMapRouter::TNodeID dest, std::vector< CMapRouter::TNodeID > &path,
			CMapRouter *router, double (*getMinimum)(CMapRouter*,CMapRouter::TNodeID,size_t)) {

  //std::cout << "Starting dijkstra minimum for start[" << src << "] dest[" << dest << "]" << std::endl;
  
  //node id -> shortest distance to every node from the initial source node 
  std::map<CMapRouter::TNodeID, double> minimums;
  std::map<CMapRouter::TNodeID, std::vector<CMapRouter::TNodeID>> paths; 
  for (auto i = 0; i < router->NodeCount(); i++) {
    minimums[router->GetSortedNodeIDByIndex(i)] = std::numeric_limits<double>::max();
    paths[router->GetSortedNodeIDByIndex(i)]    = std::vector<CMapRouter::TNodeID>();
  }
  
  //list of unevaluated nodes IDs
  std::set<CMapRouter::TNodeID> unevaluated;
  std::set<CMapRouter::TNodeID> evaluated;
  
  //initialize the first value and node
  minimums[src] = 0.0;
  unevaluated.insert(src);
  paths[src].push_back(src);
  
  //find the minimum value and path to each node in the graph from the source node
  //this part is a direct implementation of Dijkstra
  while (unevaluated.size() > 0) {

    //Find the node ID with the minimum value in the uneval set
    CMapRouter::TNodeID minNodeID = CMapRouter::InvalidNodeID;
    double minDistance = std::numeric_limits<double>::max();
    for (std::set<CMapRouter::TNodeID>::iterator iter = unevaluated.begin(); iter != unevaluated.end(); ++iter) {
      CMapRouter::TNodeID nodeID = *iter;
      double distance = minimums[nodeID];
      if (distance < minDistance) {
	minDistance = distance;
	minNodeID = nodeID;
      }
    }
    
    unevaluated.erase(minNodeID);
    
    //iterate over all edges update the mininums and paths if necessary
    for (int i = 0; i < router->GetEdgeCountByID(minNodeID); i++) {
      CMapRouter::TNodeID edgeNodeID = router->GetEdgeNodeByIndex(minNodeID, i);
      
      //use the function pointer here to get the minimum for this edge
      //which may be a distance or a time
      double edgeValue = getMinimum(router, minNodeID, i);
      
      if (!evaluated.count(edgeNodeID)) {
	double newMinimum = minimums[minNodeID] + edgeValue;
	
	//if new min is less than the stored instance
	//reset the min and path to the new min
	if (newMinimum < minimums[edgeNodeID]) {
	  minimums[edgeNodeID] = newMinimum;
	  paths[edgeNodeID] = std::vector<CMapRouter::TNodeID>();
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
  

  //std::cout << "Finished dijkstra minimum for start[" << src << "] dest[" << dest << "]" << std::endl;
  
  //set path to the minimum path to the dest node
  path = paths[dest];

  //return the minimum to the dest node
  //std::cout << "minimum value to dest node [" << minimums[dest] << "]" << std::endl;
  return minimums[dest];


}


/**
 * Used for finding the shortest path by distance
 */
double getEdgeDistance (CMapRouter *router, CMapRouter::TNodeID nodeid, size_t index) {
  return router->GetEdgeDistanceByIndex(nodeid, index);
}

/**
 * Find the shortest path using distance between nodes
 */
double CMapRouter::FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID > &path){
  return findMinimumPath(src, dest, path, this, getEdgeDistance);    
}



/**
 * Determine if the given edge is part of a bus route
 * This method depends on having a complete list of nodes
 * that are included on the bus route even if there is no stop at that node
 */
bool busRouteContainsEdge (CMapRouter *router, CMapRouter::TNodeID srcNodeID, CMapRouter::TNodeID edgeNodeID, std::string &foundRouteName) {
  
  //determine if this edge is on a bus route
  bool isBusRoute = false;
  bool foundSrc = false;
  for (auto i = 0; i < router->RouteCount(); i++) {
    std::string routeName = router->GetSortedRouteNameByIndex(i);
    
    //have to loop through the nodes instead of the stops
    //so that use the consecutive nodes
    std::vector<CMapRouter::TNodeID> nodes;
    if (router->GetRouteNodesByRouteName(routeName, nodes)) {
      
      for (auto iter = nodes.begin(); iter != nodes.end(); iter++) {
  	CMapRouter::TNodeID nodeID = *iter;
  	if (nodeID == srcNodeID) {
  	  foundSrc = true;
  	} else if (nodeID == edgeNodeID && foundSrc) {
  	  isBusRoute = true;
	  foundRouteName = routeName;
  	}
      }
      
    }
    
  }
  
  //std::cout << "[" << srcNodeID << " to " << edgeNodeID << "] found bus route [" << isBusRoute << "]" << std::endl;
  return isBusRoute;

}

/**
 * Used for finding the minimum path by time
 * 
 * Rules
 * 1. Bus travels at the speed limit of the street. If a speed limit is not specified 
 *    for the way, it is assumed that it is 25mph.
 * 2. People walk at 3mph when not on the bus.
 * 3. Each segment of the bus route (between each bus stop) takes additional 30s, 
 *    assume that the bus stops at each bus stop for 30s.
 */
double getEdgeTime (CMapRouter *router, CMapRouter::TNodeID srcNodeID, size_t index) {
  
  //get the edge information
  double edgeDistance = router->GetEdgeDistanceByIndex(srcNodeID, index);
  CMapRouter::TNodeID edgeNodeID = router->GetEdgeNodeByIndex(srcNodeID, index);

  std::string foundRouteName;
  bool isBusRoute = busRouteContainsEdge(router, srcNodeID, edgeNodeID, foundRouteName);
  
  //start with time walking
  double edgeTime = edgeDistance / 3.0;
  if (isBusRoute) {
    
    double waitTime = 0.0;
    
    //only wait if this edge ends in a route stop
    std::vector<CMapRouter::TStopID> stops;
    if (router->GetRouteStopsByRouteName(foundRouteName, stops)) {
      for (auto iter = stops.begin(); iter != stops.end(); iter++) {
	CMapRouter::TNodeID nodeID = router->GetNodeIDByStopID(*iter);
  	if (nodeID == edgeNodeID) {
  	  waitTime = 30.0 / 3600.0;
	}
      }
    }
    
    edgeTime = (edgeDistance / 25.0) + waitTime;
    //std::cout << "[" << srcNodeID << " to " << edgeNodeID << "] found bus route [" << edgeTime  << "]" << std::endl;
  } else {
    //std::cout << "[" << srcNodeID << " to " << edgeNodeID << "] only walking [" << edgeTime  << "]" << std::endl;
  }
  
  return edgeTime;
}

/**
 * Find the fastest path using routes and distance.
 * This method uses findMinimumPath and getEdgeTime instead of the geographic
 * distance between the nodes
 */
double CMapRouter::FindFastestPath(TNodeID src, TNodeID dest, std::vector< TPathStep > &path){
  std::vector< TNodeID > minNodePath; 
  double minTime = findMinimumPath(src, dest, minNodePath, this, getEdgeTime);
  
  //loop over the path of nodes and create the path
  //the represents whether the walk or bus was used to get to the node
  TNodeID prevNodeID = InvalidNodeID;
  for (auto nodeIter = minNodePath.begin(); nodeIter < minNodePath.end(); nodeIter++) {

    //if this is the first node - assume Walk to the node
    if (prevNodeID == InvalidNodeID) {
      prevNodeID = *nodeIter;
      path.push_back(std::pair<std::string, TNodeID>("Walk", *nodeIter));
      //std::cout << "Walk: " << *nodeIter << std::endl;
    } else {
      //any other node we need to determine if a bus route was used
      std::string foundRouteName = "";
      if (busRouteContainsEdge(this, prevNodeID, *nodeIter, foundRouteName)) {
	path.push_back(std::pair<std::string, TNodeID>("Bus "+foundRouteName, *nodeIter));
	//std::cout << "Bus " << foundRouteName << ": " << *nodeIter << std::endl;
      } else {
	path.push_back(std::pair<std::string, TNodeID>("Walk", *nodeIter));
	//std::cout << "Walk: " << *nodeIter << std::endl;
      }
      prevNodeID = *nodeIter;
    }
  }
    
  return minTime;
}


std::string getDMSForLocation (CMapRouter::TLocation nodeLocation) {
  double lat  = nodeLocation.first;
  double lon  = nodeLocation.second;
  
  int lonDegree  = (int)lon;
  int lonMinutes = (int) ( (lon - (double)lonDegree) * 60.0);
  int lonSeconds = (int) ( (lon - (double)lonDegree - (double)lonMinutes / 60.0) * 60.0 * 60.0 );
  
  int latDegree  = (int)lat;
  int latMinutes = (int) ( (lat - (double)latDegree) * 60.0);
  int latSeconds = (int) ( (lat - (double)latDegree - (double)latMinutes / 60.0) * 60.0 * 60.0 );

  std::string latCardinality = "S";
  if (lat >= 0) {
    latCardinality = 'N';
  }

  std::string lonCardinality = "W";
  if (lon >= 0) {
    lonCardinality = 'E';
  }
  
  std::stringstream dms;
  dms << latDegree << "d " << latMinutes << "' " << latSeconds << "\" " << latCardinality << ", "
      << lonDegree << "d " << lonMinutes << "' " << lonSeconds << "\" " << lonCardinality;
  
  return dms.str();
}

  
// EXPECT_TRUE(MapRouter.GetPathDescription(Path, Description));
// EXPECT_EQ(Description[0],"Start at 0d 0' 0\" N, 0d 0' 0\" E");
// EXPECT_EQ(Description[1],"Walk E to 0d 0' 0\" N, 1d 0' 0\" E");
// EXPECT_EQ(Description[2],"Take Bus A and get off at stop 23");
// EXPECT_EQ(Description[3],"Walk W to 1d 0' 0\" N, 0d 0' 0\" E");
// EXPECT_EQ(Description[4],"End at 1d 0' 0\" N, 0d 0' 0\" E");
bool CMapRouter::GetPathDescription(const std::vector< TPathStep > &path, std::vector< std::string > &desc) const {
  
  if (path.size() == 0) {
    return false;
  }
  
  //Start at ..
  TLocation startLocation = GetNodeLocationByID(path[0].second);    
  desc.push_back("Start at "+getDMSForLocation(startLocation));
  TLocation lastLocation = startLocation;
		 
  if (path.size() > 1) {
    bool onBus = false;
    TNodeID lastBusNodeID = InvalidNodeID;
    std::string busMode = "";
    for (int i = 1; i < path.size(); i++) {
      std::string mode = path[i].first;
      TNodeID nodeID   = path[i].second;
      TLocation nodeLocation = GetNodeLocationByID(nodeID);
      std::string dms = getDMSForLocation(nodeLocation);
      
      //std::cout << "Path[" << i << "]: node[" << nodeID << "] mode[" << mode << "]: " << dms << std::endl;
      
      if (mode == "Walk") {
	if (onBus) {

	  onBus = false;

	  //find the stop for the bus node ID
	  //this assumes 1<->1 mapping of stop ID to node ID
	  std::stringstream atStop;
	  if (lastBusNodeID != InvalidNodeID) {
	    for (auto stopIter = Stops.begin(); stopIter != Stops.end(); stopIter++) {
	      if (stopIter->second == lastBusNodeID) {
		atStop << "at stop " << stopIter->first;
	      }
	    }
	  }
	  
	  desc.push_back("Take "+busMode+" and get off "+atStop.str());
	  
	}

	//Get the bearing walking
	std::stringstream walk;
	double bearing = CalculateBearing(lastLocation.first, lastLocation.second,
					  nodeLocation.first, nodeLocation.second);
	//make sure its not negative
	double angle = fmod(bearing, 360);
	while (angle < 0.0) {
	  angle += 360.0;
	}

	//get the direction as a string
	std::string direction = BEARINGS[(int) round(std::fmod(angle, 360.0) / 45.0)];
	walk << "Walk " << direction << " to " << dms;
	desc.push_back(walk.str()); 
	
      } else {
	if (!onBus) {
	  onBus = true;
	  busMode = mode;
	}
	lastBusNodeID = nodeID;
      }
      
      lastLocation = nodeLocation;
      
    } //end of loop over paths
    
  }
  
  //End at ..
  TLocation endLocation = GetNodeLocationByID(path[path.size() - 1].second);    
  desc.push_back("End at "+getDMSForLocation(endLocation));

  // for (int i = 0; i < desc.size(); i++) {
  //   std::cout << "Desc[" << i << "]: " << desc[i] << std::endl;
  // }
		 
		 
  return true;
}
