#ifndef ASTAR
#define ASTAR

#include <cmath>

#include "binheap.hpp"
#include "map.hpp"

enum {OPEN = 1, CLOSED = 2};

int _calculate_h_cost(int test_square, int target, Map *map)
{
#if DJIKSTRA
    return 0;
#endif
    int map_width = map->hitbox->width;

    int test_square_y = test_square / map_width;
    int target_y = target / map_width;

    int y_offset = abs(test_square_y - target_y);
    int x_offset = abs((test_square - test_square_y * map_width) - (target - target_y * map_width));
    int h_cost = (x_offset + y_offset) * 90; // TODO: find out why 100 is an overestimation

    return h_cost;
}

int _calculate_g_cost(int test_square, int current_square, Map *map, int *g_costs)
{
    int g_cost;

    if (test_square == current_square - map->hitbox->width ||
        test_square == current_square + map->hitbox->width ||
        test_square == current_square - 1                  ||
        test_square == current_square + 1)
    {
        g_cost = g_costs[current_square] + 100;
    }
    else g_cost = g_costs[current_square] + 141;

    return g_cost;
}

void _check_square(int current_square, int test_square, int start, int end, Map *map, int *visited_list, int *parents, int *g_costs, int *f_costs, BinHeap *heap)
{
    // check if square is walkable and has not yet been visited
    // TODO: think more about how this map->hitbox->data should work for reals, this number thing isn't very pretty
    if (map->hitbox->data[test_square] != 1 && visited_list[test_square] != CLOSED)
    {
        // DEBUG
        map->hitbox->data[test_square] = 4;

        int g_cost = _calculate_g_cost(test_square, current_square, map, g_costs);

        // if square has never been checked, calculate its costs
        // and put it into the to-be-visited list
        if (visited_list[test_square] != OPEN)
        {
            visited_list[test_square] = OPEN;
            parents[test_square] = current_square;

            int h_cost = _calculate_h_cost(test_square, end, map);

            g_costs[test_square] = g_cost;
            f_costs[test_square] = g_cost + h_cost;

            heap->insert(test_square);
        }
        // if it has already been checked, recalculate the costs
        // of walking into it from the current_square
        // then, if the new costs are lower, change the path to
        // use the current square
        else
        {
            if (g_cost < g_costs[test_square])
            {
                parents[test_square] = current_square;

                int h_cost = _calculate_h_cost(test_square, end, map);

                g_costs[test_square] = g_cost;
                f_costs[test_square] = g_cost + h_cost;
            }
        }
    }
}

/*
   Finds a path between `start` and `end` using the A* algorithm.
   The resulting path, if one was found, is stored inside `path`,
   and its size is stored in `found_path_size`.

   @ map             :: the map in which the pathfinding will occur
   @ start           :: the starting node in the map
   @ end             :: the target node in the map
   @ path            :: a pointer that will receive the resulting path, if one is found
   @ found_path_size :: the size of the path found, if none was found, this won't change

   @ returns         :: 0 if the path was found, -1 if there is no path
 */
int find_path_astar(Map *map, int start, int end, int **path, int *found_path_size)
{
    int map_size = map->hitbox->width * map->hitbox->height;
    int *list = (int *) calloc(map_size, sizeof(int));
    int *parents = (int *) calloc(map_size, sizeof(int));
    int *f_costs = (int *) calloc(map_size, sizeof(int));
    int *g_costs = (int *) calloc(map_size, sizeof(int));
    BinHeap *heap = new BinHeap(map_size, f_costs);
    bool path_found = false;

    // add the starting node to the open list and the heap
    list[start] = OPEN;
    heap->insert(start);
    
    // start searching for a path, using the given square as the starting point
    for (;;)
    {
        int map_height = map->hitbox->height;
        int map_width = map->hitbox->width;

        // find the lowest cost square on the open list
        int current_square = heap->pop_next();
        int current_square_y = current_square / map_width;

        // if the target was found, successfully exit the loop
        if (current_square == end)
        {
            path_found = true;
            break;
        }

        // else if there are no more nodes to go to, unsucessfully exit the loop
        if (current_square == -1) break;

        // otherwise, move the square to the closed list
        list[current_square] = CLOSED;

        // calculate costs for the nearby squares
        {
            int adj_square;

            for (adj_square = current_square - map_width - 1; adj_square <= current_square - map_width + 1; adj_square++)
            {
                if (adj_square < 0) break;
                if (adj_square < (current_square_y - 1) * map_width || adj_square >= current_square_y * map_width) continue;

                _check_square(current_square, adj_square, start, end, map, list, parents, g_costs, f_costs, heap);
            }

            adj_square = current_square - 1;
            if (adj_square >= current_square_y * map_width)
            {
                _check_square(current_square, adj_square, start, end, map, list, parents, g_costs, f_costs, heap);
            }

            adj_square = current_square + 1;
            if (adj_square < (current_square_y + 1) * map_width)
            {
                _check_square(current_square, adj_square, start, end, map, list, parents, g_costs, f_costs, heap);
            }

            for (adj_square = current_square + map_width - 1; adj_square <= current_square + map_width + 1; adj_square++)
            {
                if (adj_square >= map_width * map_height) break;
                if (adj_square < (current_square_y + 1) * map_width || adj_square >= (current_square_y + 2) * map_width) continue;

                _check_square(current_square, adj_square, start, end, map, list, parents, g_costs, f_costs, heap);
            }
        }
    }

    // path not found
    if (!path_found) return -1;

    // path found, calculate its size and then recreate it
    {
        int current_square = end;
        *found_path_size = 1;
        while(current_square != start)
        {
            current_square = parents[current_square];
            *found_path_size += 1;
        }

        *path = (int *) malloc((*found_path_size) * sizeof(int));

        current_square = end;
        (*path)[0] = current_square;
        for (int i = 1; current_square != start; i++)
        {
            current_square = parents[current_square];
            (*path)[i] = current_square;
        }
    }

    // success
    return 0;
}

#endif
