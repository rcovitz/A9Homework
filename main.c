#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

typedef struct Edge {
    int from;
    int to;
    int *weights;
    struct Edge *next;
} Edge;

typedef struct {
    int vertices;
    int edges;
    int period;
    Edge *head;
    Edge *tail;
} Graph;

Edge* get_edges_from(Graph *g, int v) {
    Edge *e = g->head;
    while (e != NULL) {
        if (e->from == v) return e;
        e = e->next;
    }
    return NULL;
}

typedef struct NodeState {
    int v;          // current vertex
    int cost;       // total cost so far
    int time;       // time (hops)
} NodeState;

typedef struct PQ {
    NodeState *arr;
    int size;
} PQ;

typedef struct PathResult {
    int cost;
    int hops;
    int *path;
    int length;
} PathResult;

void pq_push(PQ *pq, NodeState ns) {
    pq->arr[pq->size++] = ns;
    for (int i = pq->size - 1; i > 0; i--) {
        if (pq->arr[i].cost < pq->arr[i-1].cost) {
            NodeState t = pq->arr[i];
            pq->arr[i] = pq->arr[i-1];
            pq->arr[i-1] = t;
        }
    }
}

NodeState pq_pop(PQ *pq) {
    return pq->arr[--pq->size];
}

PathResult dijkstra(Graph *g, int start, int end) {

    int best[g->vertices][g->period];
    int prev[g->vertices][g->period];
    bool done[g->vertices][g->period];

    for(int i=0;i<g->vertices;i++)
        for(int t=0;t<g->period;t++)
            best[i][t]=INT_MAX, prev[i][t]=-1, done[i][t]=false;

    PQ pq = { malloc(100000*sizeof(NodeState)), 0 };

    best[start][0]=0;
    pq_push(&pq,(NodeState){start,0,0});

    while(pq.size>0){

        NodeState cur=pq_pop(&pq);
        int u=cur.v, t=cur.time % g->period;

        if(done[u][t])continue;
        done[u][t]=true;

        Edge *e=g->head;
        while(e){
            if(e->from==u){
                int nt=(t+1)%g->period;
                int w=e->weights[t];
                int nc=cur.cost+w;

                if(nc < best[e->to][nt]){
                    best[e->to][nt]=nc;
                    prev[e->to][nt]=u;
                    pq_push(&pq,(NodeState){e->to,nc,cur.time+1});
                }
            }
            e=e->next;
        }
    }

    // find best arrival state for END
    int best_t=0;
    for(int t=1;t<g->period;t++)
        if(best[end][t]<best[end][best_t]) best_t=t;

    // reconstruct path
    int *path=malloc(g->vertices*sizeof(int));
    int len=0, cur=end, t=best_t;
    while(cur!=-1){
        path[len++]=cur;
        int p=prev[cur][t];
        t=(t-1+g->period)%g->period;
        cur=p;
    }
    for(int i=0;i<len/2;i++){int x=path[i];path[i]=path[len-1-i];path[len-1-i]=x;}

    return (PathResult){ best[end][best_t], -1/*unused*/, path, len };
}


Graph* read_file(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    int vertices, period;
    fscanf(file, "%d %d", &vertices, &period);

    Graph *g = malloc(sizeof(Graph));
    g->vertices = vertices;
    g->period = period;
    g->edges = 0;
    g->head = NULL;
    g->tail = NULL;

    while (true) {
        int from, to;
        if (fscanf(file, "%d %d", &from, &to) != 2)
            break;

        Edge *e = malloc(sizeof(Edge));
        e->from = from;
        e->to = to;

        e->weights = malloc(period * sizeof(int));
        for (int i = 0; i < period; i++) fscanf(file, "%d", &e->weights[i]);

        
        e->next = NULL;
        if (g->head == NULL) {
            g->head = e;
            g->tail = e;
        } else {
            g->tail->next = e;
            g->tail = e;
        }
        g->edges++;
    }

    printf("Vertices: %d\n", g->vertices);
    printf("Edges: %d\n", g->edges);
    printf("Period: %d\n", g->period);

    fclose(file);
    return g;
}

void print_graph(Graph *graph) {
    Edge *edge = graph->head;
    if (edge == NULL) {
        return;
    }
    while (edge != NULL) {
        printf("%d->%d  w: ", edge->from, edge->to);
        for (int i = 0; i < graph->period-1; i++) {
            printf("%d,", edge->weights[i]);
        }
        printf("%d", edge->weights[graph->period-1]);
        printf("\n");
        edge = edge->next;
    }
}

void free_graph(Graph *graph) {
    Edge *edge = graph->head;
    while (edge != NULL) {
        Edge *next = edge->next;
        free(edge->weights);
        free(edge);
        edge = next;
    }
    free(graph);
}


int main(int argc, char *argv[]) {


    Graph *graph = read_file("graph3.txt");
    
    print_graph(graph);

    PathResult r = dijkstra(graph, 0, 3);

    printf("Best cost: %d\n", r.cost);
    printf("Path: ");
    for (int i = 0; i < r.length; i++)
        printf("%d ", r.path[i]);
    printf("\n");

    free(r.path);


    free_graph(graph);
    return 0;
}
