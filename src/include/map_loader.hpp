#ifndef MAP_LOADER_H
#define MAP_LOADER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <system_error>
#include <string>
#include <vector>

#include "structs.hpp"

std::string TrimLeft(const std::string& str);
std::string TrimRight(const std::string& str);
std::string Trim(const std::string& str);
std::vector< std::string > Explode(const std::string& str, const std::string& separator);

/**
 * \brief Tries to load the map list
 */
bool LoadMapList( std::vector<std::string>& map_list )
{
    std::ifstream maps_file_stream;

    //try to load map list file
    try
    {
        maps_file_stream.open( path_data + "map_list" );

        if(maps_file_stream.is_open())
        {
            std::string line;
            //read each line
            while(getline(maps_file_stream, line))
            {
                map_list.push_back(line);
            }
        }

        //ALLWAYS close your damn files, although maps_file_stream destructor would close all when its get out of scope
        maps_file_stream.close();

        map_list.shrink_to_fit();
    }
    catch(const std::exception &e)
    {
        std::cout << "Failed to open map list file - " << e.what() << " ... \n" << "Terminating application" << std::endl;
        return false;//will kill the program as the program was unable to initialize its basic configuration
    }

    return true;
}

/**
* \brief Load a map from its file
*/
bool LoadMap(std::string map_name, MapData& data)
{
    std::ifstream map_file_stream;

    //try to load map list file
    try
    {
        map_file_stream.open( path_map + map_name + file_extension_map);

        if(map_file_stream.is_open())
        {
            data.name = map_name;

            std::string line;
            std::vector<std::string> line_sections;
            //read header
            //read search type
            map_file_stream >> line >> line;//jump to second word
            if(line == "octile")
            {
                data.search_type = SEARCH_TYPE::OCTILE_NOT_CORNER;
                data.heuristic_type = HEURISTIC_TYPE::OCTILE;
            }
            else if(line == "octile_high_8")
            {
                data.search_type = SEARCH_TYPE::OCTILE_HIGH_8;
                data.heuristic_type = HEURISTIC_TYPE::OCTILE;
            }
            else if(line == "octile_high_64")
            {
                data.search_type = SEARCH_TYPE::OCTILE_HIGH_64;
                data.heuristic_type = HEURISTIC_TYPE::OCTILE;
            }

            //read height
            map_file_stream >> line >> line;
            data.map_height = std::stoul(line);
            //read width
            map_file_stream >> line >> line;
            data.map_width = std::stoul(line);

            map_file_stream >> line;//we ignore the 4th line

            data.map.clear();//for safety

            //read the map
            int buffer_size = 0;
            while( map_file_stream >> line )
            {
                for(unsigned int i = 0; i < line.size(); i++)
                {
                    if(line[i] == '@' || line[i] == 'T')
                        data.map.push_back(0);
                    else if(line[i] == '.')
                    {
                        data.map.push_back(1);
                        buffer_size++;
                    }
                }
            }
            data.path_buffer.clear();//clear buffer vector

            //read entire map, now we read its bench data
            map_file_stream.close();//close previous file

            map_file_stream.open( path_map + map_name + file_extension_map_scenario);

            if(map_file_stream.is_open())
            {
                std::getline(map_file_stream, line);//we ignore the first line as we do not need it

                data.benchmarks.clear();
                int counter = 0;
                while(std::getline(map_file_stream, line) && counter < max_benchmarks)
                {
                    std::istringstream line_stream(line);
                    MapBenchmark benchmark;

                    //we read the entry
                    line_stream >> line >> line >> line >> line >> line;//I know this is not the best method

                    benchmark.start_x = std::stoul(line);
                    line_stream >> line;
                    benchmark.start_y = std::stoul(line);

                    line_stream >> line;
                    benchmark.target_x = std::stoul(line);
                    line_stream >> line;
                    benchmark.target_y = std::stoul(line);

                    line_stream >> line;//read result
                    benchmark.expected_min_path_cost = std::stof(line);

                    data.benchmarks.push_back( benchmark );
                    counter++;
                }

            }//bench open

        }//map open

    }
    catch(const std::exception &e)
    {
        std::cout << "Failed to open map file - " << e.what() << " ... \n" << "Terminating application" << std::endl;
        return false;//will kill the program as the program was unable to initialize its basic configuration
    }

    return true;
}

//string manipulation


std::string Trim(const std::string& str)
{
    return TrimLeft(TrimRight(str));
}

std::string TrimLeft(const std::string& str)
{
    std::size_t position = str.find_first_not_of(" ");

    if(position == std::string::npos)//empty str or only " "
        return "";

    //will remove everything prior to the position
    return str.substr( position );
}

std::string TrimRight(const std::string& str)
{
    std::size_t position = str.find_last_not_of(" ");

    if(position == std::string::npos)//empty str or only " "
        return "";

    //will remove everything after to the position + 1 (+1 because the position is an index and substr needs a length, so from 0 to position+1)
    return str.substr( 0, position+1 );
}

/**
* \brief Returns a string vector with the given string separated by the separator
*/
std::vector< std::string > Explode(const std::string& str, const std::string& separator)
{
        std::vector< std::string > result_string;
    	std::string::size_type position;
        std::string str_auxilia = str;

    	str_auxilia = Trim(str_auxilia);

        while( (position = str_auxilia.find(separator)) != std::string::npos)//found next cut area
        {
            result_string.push_back( str_auxilia.substr(0, position) );

            str_auxilia = str_auxilia.substr( position + 1);
        }

        //stores the last section as it will not run the loop
        result_string.push_back( str_auxilia );

        return result_string;
}

#endif //MAP_LOADER_H
