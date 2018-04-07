from collections import defaultdict

## Graph Representation for edge with weight
class Graph:

    def __init__(self, vertices):
        self.V = vertices  # No. of vertices
        self.graph = []  # default dictionary
        # to store graph

    # function to add an edge to graph
    def addEdge(self, u, v, w):
        self.graph.append([u, v, w])

    # A utility function to find set of an element i
    # (uses path compression technique)
    def find(self, parent, i):
        if parent[i] == i:
            return i
        return self.find(parent, parent[i])

    # A function that does union of two sets of x and y
    # (uses union by rank)
    def union(self, parent, rank, x, y):
        xroot = self.find(parent, x)
        yroot = self.find(parent, y)

        # Attach smaller rank tree under root of
        # high rank tree (Union by Rank)
        if rank[xroot] < rank[yroot]:
            parent[xroot] = yroot
        elif rank[xroot] > rank[yroot]:
            parent[yroot] = xroot

        # If ranks are same, then make one as root
        # and increment its rank by one
        else:
            parent[yroot] = xroot
            rank[xroot] += 1


# Function to calculate MST using Kruskal's algorithm
def KruskalMST(self):
    result = []  # This will store the resultant MST

    i = 0  # An index variable, used for sorted edges
    e = 0  # An index variable, used for result[]

    # Step 1:  Sort all the edges in non-decreasing
    # order of their weight.  If we are not allowed to change the
    # given graph, we can create a copy of graph

    self.graph = sorted(self.graph, key=lambda item: item[2])

    parent = [];
    rank = []

    # Create V subsets with single elements
    for node in range(self.V):
        parent.append(node)
        rank.append(0)

    # Number of edges to be taken is equal to V-1
    while e < self.V - 1:

        # Step 2: Pick the smallest edge and increment
        # the index for next iteration
        u, v, w = self.graph[i]
        i = i + 1
        x = self.find(parent, u)
        y = self.find(parent, v)

        # If including this edge does't cause cycle,
        # include it in result and increment the index
        # of result for next edge
        if x != y:
            e = e + 1
            result.append([u, v, w])
            self.union(parent, rank, x, y)
            # Else discard the edge

    return result

# Class to construct graph with result from MST and use DFS for traversal
class Graph2:

    # Constructor
    def __init__(self):

        # default dictionary to store graph
        self.graph = defaultdict(list)

    # function to add an edge to graph
    def addEdge(self, u, v, w):
        self.graph[u].append(v)
        self.graph[v].append(u)

    # A function used by DFS
    def DFSUtil(self, v, visited, out):
        # print(out)
        # Mark the current node as visited and print it
        visited[v] = True
        # print(v)
        out.append(v)
        # Recur for all the vertices adjacent to this vertex
        for i in self.graph[v]:
            if visited[i] == False:
                self.DFSUtil(i, visited, out)

    # The function to do DFS traversal. It uses
    # recursive DFSUtil()
    def DFS(self, v):
        out = []
        # Mark all the vertices as not visited
        visited = [False] * (len(self.graph) + 1)
        # Call the recursive helper function to print
        # DFS traversal
        self.DFSUtil(v, visited, out)
        return out


if __name__ == '__main__':
    # Construct graph with weight and compute MST

    g = Graph(9)
    g.addEdge(0, 1, 4)
    g.addEdge(0, 7, 8)
    g.addEdge(1, 2, 8)
    g.addEdge(1, 7, 11)
    g.addEdge(2, 3, 7)
    g.addEdge(2, 8, 2)
    g.addEdge(2, 5, 4)
    g.addEdge(3, 4, 9)
    g.addEdge(3, 5, 14)
    g.addEdge(4, 5, 10)
    g.addEdge(5, 6, 2)
    g.addEdge(6, 7, 1)
    g.addEdge(6, 8, 6)
    g.addEdge(7, 8, 7)

    print("Original Graph with Weight: ", g.graph)

    # MST
    MST = KruskalMST(g)
    print("MST: ", MST)

    g2 = Graph2()

    for i in MST:
        g2.addEdge(i[0], i[1], i[2])
        # print(g2.graph)
    # print(g2.graph)
    traversal = g2.DFS(2)
    print("Source: 2, "+"DFS Traversal: ", traversal)