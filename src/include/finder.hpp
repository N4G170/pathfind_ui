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

/**
* \brief Process a node and checks its neighbors storing them if valid
*/
bool FinderStep(SearchData& search_data, MapData& map_data, std::vector<DrawData>& draw_data_grid, MapBenchmark& benchmark);

#endif //FINDER_H
