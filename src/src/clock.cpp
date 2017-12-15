#include "clock.hpp"

Clock::Clock()
{
    m_id_counter = 0;
}

Clock::~Clock()
{

}

//initialize static variable
std::unique_ptr<Clock> Clock::s_instance;

/**
 * \brief Get a std::unique_ptr to the singleton instance
 */
std::unique_ptr<Clock>& Clock::Instance()
{
    //possibly move initialization to a new function and CALL IT at the beginning of the program
    //and remove this check, as it will run every time Instance() is called
    if(!s_instance)//does not exists
    {
        s_instance.reset(new Clock);
    }

    return s_instance;
}

/**
 * \brief Starts a new timer
 * \return Id to the new timer running
 */
unsigned long Clock::StartClock()
{
    //lock->increase->unlock
    m_mutex.lock();

    m_id_counter++;
    unsigned long new_id = m_id_counter;

    m_mutex.unlock();


    m_clocks.emplace( new_id, std::chrono::steady_clock::now());

    return std::move( new_id );
}

/**
 * \brief Stops a specific timer
 * \return std::string with the time in miliseconds
 */
std::string Clock::StopAndReturnClock(unsigned long id)
{
    if(m_clocks.find(id) == m_clocks.end())//id does not exist
        return "Invalid clock ID - "+std::to_string(id);

    auto diff_time = std::chrono::steady_clock::now() - m_clocks[id];

    //remove the clock fom the map as it will not be used again
    m_clocks.erase(id);

    return std::to_string(std::chrono::duration <double, std::milli> (diff_time).count());
}
