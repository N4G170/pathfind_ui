#ifndef CLOCK_H
#define CLOCK_H

#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <map>

/**
 * \brief Basic SINGLETON class to manage a small clock to be used by Euler Problems functions
 */
class Clock
{
    public:
        virtual ~Clock();

        /**
         * \brief Get a std::unique_ptr to the singleton instance
         */
        static std::unique_ptr<Clock>& Instance();

        //we delete the copy constructor and the assign operator to avoid the accidental copy of our main instance
        Clock(Clock const&)     = delete;
        void operator=(Clock const&)    = delete;

        /**
         * \brief Starts a new timer
         * \return Id to the new timer running
         */
        unsigned long StartClock();

        /**
         * \brief Stops a specific timer
         * \return std::string with the time in miliseconds
         */
        std::string StopAndReturnClock(unsigned long id);

    private:
        /**
         * \brief Private constructor to avoid "external" instance creation
         */
        Clock();//private default constructor

        /**
         * \brief std::unique_ptr to the static instance
         */
        static std::unique_ptr<Clock> s_instance;

        /**
         * \brief Counter to generate ids for new timers
         */
        unsigned long m_id_counter;

        /**
         * \brief std::map storing the timers currently running
         */
        std::map<unsigned long, std::chrono::time_point<std::chrono::steady_clock> > m_clocks;

        /**
         * \brief std::mutex to control id counter access
         */
        std::mutex m_mutex;
};


#endif // CLOCK_H
