#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// Used for each directed edge
typedef struct Edge {
    int to;                // vertex the edge points to
    int *weights;          // array of length period to store the weights
    struct Edge *next;     // pointer to the next edge, used in the adjacency list
} Edge;

// Used to store the entire graph
typedef struct {
    int vertices;          // number of vertices total
    int period;            // period, or number of weights per edge
    Edge **adj;            // array of edges that have linked lists for which vertices they point to
} Graph;

// Used in the priority queue for the shortest path algo
typedef struct {
    int vertex;             // current vertex
    int cost;               // total cost to reach the vertex
    int time;               // time step when the vertex was reached
} NodeState;

// Used for the priority queue
typedef struct {
    NodeState *arr;         // array of nodes, the heap
    int size;               // size of the heap
    int capacity;           // capacity of the heap
} MinHeap;

// Used to store the shortest path for return
typedef struct {
    int *path;              // array of vertices in the shortest path
    int length;             // length of the shortest path
} PathResult;

// Creates a new heap to do the priority queue
MinHeap* create_heap(int capacity) {
    MinHeap *heap = malloc(sizeof(MinHeap));
    heap->arr = malloc(capacity * sizeof(NodeState));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Swaps two nodes in the heap
void swap(NodeState *a, NodeState *b) {
    NodeState temp = *a;
    *a = *b;
    *b = temp;
}

// Pushes a node to the heap
void heap_push(MinHeap *heap, NodeState ns) {
    // if the heap is full, double capacity
    if (heap->size >= heap->capacity) {
        heap->capacity *= 2;
        heap->arr = realloc(heap->arr, heap->capacity * sizeof(NodeState));
    }
    // Increase size
    int i = heap->size++;

    // Move the node to the correct position, heapify_up
    heap->arr[i] = ns;
    while (i > 0) {
        int parent = (i-1)/2;
        if (heap->arr[parent].cost <= heap->arr[i].cost) break;
        swap(&heap->arr[parent], &heap->arr[i]);
        i = parent;
    }
}

// Pops a node from the heap and returns
NodeState heap_pop(MinHeap *heap) {
    NodeState top = heap->arr[0];
    heap->arr[0] = heap->arr[--heap->size];
    int i = 0;
    // Move node to the correct position, heapify_down
    while (true) {
        int left = 2*i + 1;
        int right = 2*i + 2;
        int smallest = i;
        if (left < heap->size && heap->arr[left].cost < heap->arr[smallest].cost) smallest = left;
        if (right < heap->size && heap->arr[right].cost < heap->arr[smallest].cost) smallest = right;
        if (smallest == i) break;
        swap(&heap->arr[i], &heap->arr[smallest]);
        i = smallest;
    }
    return top;
}

// Simple function to free the heap
void free_heap(MinHeap *heap) {
    free(heap->arr);
    free(heap);
}

// Reads the file to a graph onject
Graph* read_file(const char *filename) {
    // Opening file
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    // Reading vertices and period from first line
    int vertices, period;
    if (fscanf(f, "%d %d", &vertices, &period) != 2) { fclose(f); return NULL; }

    // Creating graph using vertices and period
    Graph *graph = malloc(sizeof(Graph));
    graph->vertices = vertices;
    graph->period = period;
    graph->adj = calloc(vertices, sizeof(Edge*));

    // Reading edges line by line
    while (true) {
        // Read from and to for the edge
        int from, to;
        if (fscanf(f, "%d %d", &from, &to) != 2) break;

        // Creating edge
        Edge *edge = malloc(sizeof(Edge));
        edge->weights = malloc(period * sizeof(int));
        // Adding weights to the edge
        for (int i = 0; i < period; i++) fscanf(f, "%d", &edge->weights[i]);
        edge->to = to;

        // Adding edge to the graph
        edge->next = graph->adj[from];
        graph->adj[from] = edge;
    }

    fclose(f);
    return graph;
}


// Function to free the graph
void free_graph(Graph *graph) {
    // Freeing edges from the graph
    for (int i = 0; i < graph->vertices; i++) {
        Edge *edge = graph->adj[i];
        while (edge) {
            Edge *next = edge->next;
            free(edge->weights);
            free(edge);
            edge = next;
        }
    }
    // Freeing graph
    free(graph->adj);
    free(graph);
}

// Finds the shortest path from vertex start to vertex end using algo similar to Dijkstra
PathResult shortest_path(Graph *graph, int start, int end) {
    // Setting up variables
    int vertices = graph->vertices;
    int period = graph->period;

    // Creating best and prev arrays
    int **best = malloc(vertices * sizeof(int*));
    int **prev = malloc(vertices * sizeof(int*));
    bool **done = malloc(vertices * sizeof(bool*));

    // Allocating and filling arrays with basic values
    for (int i = 0; i < vertices; i++) {
        best[i] = malloc(period * sizeof(int));
        prev[i] = malloc(period * sizeof(int));
        done[i] = malloc(period * sizeof(bool));
        for (int t = 0; t < period; t++) {
            best[i][t] = INT_MAX;
            prev[i][t] = -1;
            done[i][t] = false;
        }
    }

    // Creating priority queue
    MinHeap *pq = create_heap(1000);
    // Setting start time to 0
    best[start][0] = 0;
    // Adding start to the queue
    heap_push(pq, (NodeState){start,0,0});

    // Looping until the queue is empty
    while (pq->size > 0) {
        // Popping current node
        NodeState currentNode = heap_pop(pq);
        int currentVertex = currentNode.vertex;
        int currentTime = currentNode.time % period;
        // If the node has already been visited, continue
        if (done[currentVertex][currentTime]) continue;
        // Marking the node as visited
        done[currentVertex][currentTime] = true;

        // Looping through the edges
        for (Edge *edge = graph->adj[currentVertex]; edge; edge = edge->next) {
            // Getting next time and cost
            int nextTime = (currentTime + 1) % period;
            int nextCost = currentNode.cost + edge->weights[currentTime];
            // If the next cost is less than the best cost, update the best cost
            if (nextCost < best[edge->to][nextTime]) {
                best[edge->to][nextTime] = nextCost;
                prev[edge->to][nextTime] = currentVertex;
                heap_push(pq, (NodeState){edge->to, nextCost, currentNode.time+1});
            }
        }
    }

    // Find best arrival time
    int bestArrivalTime = 0;
    for (int t = 1; t < period; t++)
        if (best[end][t] < best[end][bestArrivalTime]) bestArrivalTime = t;

    // Reconstructing the path using best arival time
    int *path = malloc(vertices * sizeof(int));
    int len = 0;
    int current = end;
    int t = bestArrivalTime;
    // Looping through the path
    while (current != -1) {
        path[len++] = current;
        int p = prev[current][t];
        t = (t - 1 + period) % period;
        current = p;
    }
    // Reversing the path
    for (int i = 0; i < len/2; i++) {
        int tmp = path[i];
        path[i] = path[len-1-i];
        path[len-1-i] = tmp;
    }

    // Freeing and cleaning up the used memory
    for (int i = 0; i < vertices; i++) {
        free(best[i]);
        free(prev[i]);
        free(done[i]);
    }
    free(best);
    free(prev);
    free(done);

    free_heap(pq);

    return (PathResult){path, len};
}

// Main function, reads input loop and calls shortest path function
int main(int argc, char *argv[]) {
    // Reading file into graph
    Graph *graph = read_file(argv[1]);
    if (!graph) return EXIT_FAILURE;

    // Looping through input
    int from, to;
    while (scanf("%d %d", &from, &to) == 2) {

        PathResult result = shortest_path(graph, from, to);
            for (int i = 0; i < result.length - 1; i++) printf("%d ", result.path[i]);
            printf("%d\n", result.path[result.length - 1]);
        free(result.path);
    }

    // Freeing graph to prevent memory leak
    free_graph(graph);
    return EXIT_SUCCESS;
}
