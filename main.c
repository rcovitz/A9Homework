#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

typedef struct Edge {
    int to;
    int *weights;          // array of length 'period'
    struct Edge *next;
} Edge;

typedef struct {
    int vertices;
    int period;
    Edge **adj;            // adjacency list
} Graph;

typedef struct {
    int v;      // vertex
    int cost;   // total cost
    int time;   // time step
} NodeState;

typedef struct {
    NodeState *arr;
    int size;
    int capacity;
} MinHeap;

MinHeap* create_heap(int capacity) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->arr = malloc(capacity * sizeof(NodeState));
    h->size = 0;
    h->capacity = capacity;
    return h;
}

void swap(NodeState *a, NodeState *b) {
    NodeState tmp = *a;
    *a = *b;
    *b = tmp;
}

void heap_push(MinHeap *h, NodeState ns) {
    if (h->size >= h->capacity) {
        h->capacity *= 2;
        h->arr = realloc(h->arr, h->capacity * sizeof(NodeState));
    }
    int i = h->size++;
    h->arr[i] = ns;
    while (i > 0) {
        int parent = (i-1)/2;
        if (h->arr[parent].cost <= h->arr[i].cost) break;
        swap(&h->arr[parent], &h->arr[i]);
        i = parent;
    }
}

NodeState heap_pop(MinHeap *h) {
    NodeState top = h->arr[0];
    h->arr[0] = h->arr[--h->size];
    int i = 0;
    while (1) {
        int left = 2*i + 1;
        int right = 2*i + 2;
        int smallest = i;
        if (left < h->size && h->arr[left].cost < h->arr[smallest].cost) smallest = left;
        if (right < h->size && h->arr[right].cost < h->arr[smallest].cost) smallest = right;
        if (smallest == i) break;
        swap(&h->arr[i], &h->arr[smallest]);
        i = smallest;
    }
    return top;
}

void free_heap(MinHeap *h) {
    free(h->arr);
    free(h);
}

typedef struct {
    int cost;
    int *path;
    int length;
} PathResult;

Graph* read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    int vertices, period;
    if (fscanf(f, "%d %d", &vertices, &period) != 2) { fclose(f); return NULL; }

    Graph *g = malloc(sizeof(Graph));
    g->vertices = vertices;
    g->period = period;
    g->adj = calloc(vertices, sizeof(Edge*));

    while (1) {
        int from, to;
        if (fscanf(f, "%d %d", &from, &to) != 2) break;

        Edge *e = malloc(sizeof(Edge));
        e->weights = malloc(period * sizeof(int));
        for (int i = 0; i < period; i++) fscanf(f, "%d", &e->weights[i]);
        e->to = to;

        e->next = g->adj[from];
        g->adj[from] = e;
    }

    fclose(f);
    return g;
}

void free_graph(Graph *g) {
    for (int i = 0; i < g->vertices; i++) {
        Edge *e = g->adj[i];
        while (e) {
            Edge *next = e->next;
            free(e->weights);
            free(e);
            e = next;
        }
    }
    free(g->adj);
    free(g);
}

PathResult dijkstra(Graph *g, int start, int end) {
    int vertices = g->vertices;
    int period = g->period;

    int **best = malloc(vertices * sizeof(int*));
    int **prev = malloc(vertices * sizeof(int*));
    bool **done = malloc(vertices * sizeof(bool*));

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

    MinHeap *pq = create_heap(1000);
    best[start][0] = 0;
    heap_push(pq, (NodeState){start,0,0});

    while (pq->size > 0) {
        NodeState cur = heap_pop(pq);
        int u = cur.v;
        int t = cur.time % period;
        if (done[u][t]) continue;
        done[u][t] = true;

        for (Edge *e = g->adj[u]; e; e = e->next) {
            int nt = (t + 1) % period;
            int nc = cur.cost + e->weights[t];
            if (nc < best[e->to][nt]) {
                best[e->to][nt] = nc;
                prev[e->to][nt] = u;
                heap_push(pq, (NodeState){e->to, nc, cur.time+1});
            }
        }
    }

    // Find best arrival time
    int best_t = 0;
    for (int t = 1; t < period; t++)
        if (best[end][t] < best[end][best_t]) best_t = t;

    // Reconstruct path
    int *path = malloc(vertices * sizeof(int));
    int len = 0;
    int cur = end, t = best_t;
    while (cur != -1) {
        path[len++] = cur;
        int p = prev[cur][t];
        t = (t - 1 + period) % period;
        cur = p;
    }
    for (int i = 0; i < len/2; i++) {
        int tmp = path[i];
        path[i] = path[len-1-i];
        path[len-1-i] = tmp;
    }

    int final_cost = best[end][best_t];  // save the value first

    // cleanup
    for (int i = 0; i < vertices; i++) {
        free(best[i]);
        free(prev[i]);
        free(done[i]);
    }
    free(best);
    free(prev);
    free(done);

    free_heap(pq);

    return (PathResult){final_cost, path, len};

}

int main(int argc, char *argv[]) {
    Graph *g = read_file(argv[1]);
    if (!g) return 1;

    int from, to;
    while (scanf("%d %d", &from, &to) == 2) {

        PathResult r = dijkstra(g, from, to);
            for (int i = 0; i < r.length - 1; i++) printf("%d ", r.path[i]);
            printf("%d\n", r.path[r.length - 1]);
        free(r.path);
    }

    free_graph(g);
    return 0;
}
