#ifndef MAPROUTER_H
#define MAPROUTER_H

#include <vector>
#include <map>
#include <istream>

class CMapRouter{
    public:
        using TNodeID = unsigned long;
        using TStopID = unsigned long;
        using TLocation = std::pair<double, double>;
        using TPathStep = std::pair<std::string, TNodeID>;

        static const TNodeID InvalidNodeID;

    private:
      struct Edge {
        double Distance;
        TNodeID DestNodeID;
      };

      struct Node {
        TNodeID NodeID;
	TLocation Location;
        std::vector<Edge> Edges;
      };



      std::vector<Node> Nodes;
      std::map<TNodeID,Node> NodeTranslation;
      std::vector<TNodeID> SortedNodeIDs;
      std::map<std::string, std::vector<TStopID>> RouteStopIDs;
      std::map<std::string, std::vector<TNodeID>> RouteNodeIDs;
      std::map<TStopID, TNodeID> Stops;
      std::vector<TStopID> SortedStopIDs;
      std::vector<std::string> SortedRouteNames;



    public:
        CMapRouter();
        ~CMapRouter();

        static double HaversineDistance(double lat1, double lon1, double lat2, double lon2);
        static double CalculateBearing(double lat1, double lon1,double lat2, double lon2);

        bool LoadMapAndRoutes(std::istream &osm, std::istream &stops, std::istream &routes);
        size_t NodeCount() const;
        TNodeID GetSortedNodeIDByIndex(size_t index) const;
        TLocation GetSortedNodeLocationByIndex(size_t index) const;
        TLocation GetNodeLocationByID(TNodeID nodeid) const;
        TNodeID GetNodeIDByStopID(TStopID stopid) const;
        size_t RouteCount() const;
        std::string GetSortedRouteNameByIndex(size_t index) const;
        bool GetRouteStopsByRouteName(const std::string &route, std::vector< TStopID > &stops);

	/* Added to reduce amount of copy and paste code */
	size_t GetEdgeCountByID(TNodeID nodeid) const;
	TNodeID GetEdgeNodeByIndex(TNodeID nodeid, size_t index) const;
	double GetEdgeDistanceByIndex(TNodeID nodeid, size_t index) const;
	bool GetRouteNodesByRouteName(const std::string &route, std::vector< TNodeID > &stops);
	
        double FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID > &path);
        double FindFastestPath(TNodeID src, TNodeID dest, std::vector< TPathStep > &path);
        bool GetPathDescription(const std::vector< TPathStep > &path, std::vector< std::string > &desc) const;
};

#endif
