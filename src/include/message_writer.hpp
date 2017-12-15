#ifndef MESSAGE_WRITER_H
#define MESSAGE_WRITER_H

#include <string>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>

/**
 * \brief Singleton class controlling a basic output writer to the console and/or output box
 */
class MessageWriter
{
    public:
        virtual ~MessageWriter();

        /**
         * \brief Singleton instance
         */
        static std::unique_ptr<MessageWriter>& Instance();

        //we delete the copy constructor and the assign operator to avoid the accidental copy of our mais instance
        MessageWriter(MessageWriter const&)     = delete;
        void operator=(MessageWriter const&)    = delete;

        /**
         * \brief Stores a message in console queue (no line break)
         */
        void WriteToConsole(const std::string& message);
        /**
         * \brief Stores a message in console queue (with line break)
         */
        void WriteLineToConsole(const std::string& message);

    private:
        /**
         * \brief Private constructor to avoid "external" instance creation
         */
        MessageWriter();//private default constructor

        /**
         * \brief Reads queue and prints element to console
         */
        void ProcessConsoleQueue();


        static std::unique_ptr<MessageWriter> s_instance;

        //thread related
        bool m_running;

        /**
         * \brief Console message queue, to be printed
         */
        std::queue< std::string > m_console_message_queue;

        /**
         * \brief Thread reading from console queue and printing to console
         */
        std::thread m_console_writer;

        /**
         * \brief Mutex controlling read/write access to console queue
         */
        std::mutex m_console_mutex;

};


#endif // MESSAGE_WRITER_H
