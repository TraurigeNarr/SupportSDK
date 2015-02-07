#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "GraphsAPI.h"

namespace SDK
	{
	namespace Graphs
		{
		class GraphNode;
		class Connection;
		typedef std::vector<Connection> Connections;
		typedef std::vector<const Connection*const> ConnectionConstPtrs;
		typedef std::unique_ptr<GraphNode> NodePtr;

		class Graph
			{
			private:
				std::vector<NodePtr>	m_nodes;
				Connections						m_connections;
				int										m_next_index;

			public:
				GRAPHS_API Graph();
				GRAPHS_API virtual ~Graph();

				/*
				Returns an array of connections outgoing from given node
				*/
				GRAPHS_API ConnectionConstPtrs	GetConnections(GraphNode* ip_node) const;

				GRAPHS_API GraphNode*		AddNode(NodePtr ip_node);
				GRAPHS_API void					RemoveNode(GraphNode* ip_node);
				GRAPHS_API GraphNode*		GetNode(int i_node) const;

				GRAPHS_API Connection*	AddConnection(GraphNode* ip_from, GraphNode* ip_to, int i_cost);
				GRAPHS_API void					RemoveConnection(Connection* ip_connection);

				/*
				TODO: initialize graph after adding nodes/connecitons to store data
					more accurate
				*/
				GRAPHS_API void					Initialize();
			};

		} // Graphs
	} // SDK

#endif