#ifndef FINDER_H
#define FINDER_H

#include <vector>
#include <map>
#include <queue>
#include <set>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <algorithm>
#include "structs.hpp"
#include "utils.hpp"
#include "ui_structs.hpp"
#include "map_renderer.hpp"

/**
* \brief Process a node and checks its neighbors storing them if valid
*/
bool FinderStep(SearchData& search_data, MapData& map_data, MapRenderer* map_renderer, MapBenchmark& benchmark);

#endif //FINDER_H
