#include <iostream>
#include <chrono>

#include "message_writer.hpp"
//needed to be able to write 0.5s as a valid time
using namespace std::chrono_literals;

MessageWriter::MessageWriter()
{
    m_running = true;

    m_console_writer = std::thread{ [this](){ this->ProcessConsoleQueue(); } };
}

MessageWriter::~MessageWriter()
{
    m_running = false;
    m_console_writer.join();
}

//init static var
std::unique_ptr<MessageWriter> MessageWriter::s_instance;

std::unique_ptr<MessageWriter>& MessageWriter::Instance()
{
    //possibly move initialization to a new function and CALL IT at the beginning of the program
    //and remove this check, as it will run every time Instance() is called
    if(!s_instance)//does not exists
    {
        s_instance.reset(new MessageWriter);
    }

    return s_instance;
}

void MessageWriter::ProcessConsoleQueue()
{
    //perhaps change this to conditional_vars, that way we can avoid while loops and thread sleeps
    while(m_running)
    {
        if(!m_console_message_queue.empty())
        {
            m_console_mutex.lock();

            std::cout << m_console_message_queue.front();
            m_console_message_queue.pop();

            m_console_mutex.unlock();
        }
        else
        {
            //if there is nothing to write, sleep for 0.5 seconds
            //this is not the best implementation, from what I read, condition vars are the way to go
            std::this_thread::sleep_for(0.5s);//0.5s is not that much
        }

    }
}

void MessageWriter::WriteToConsole(const std::string& message)
{
    m_console_mutex.lock();
    m_console_message_queue.push(message);
    m_console_mutex.unlock();
}

void MessageWriter::WriteLineToConsole(const std::string& message)
{
    m_console_mutex.lock();
    m_console_message_queue.push(message + "\n");
    m_console_mutex.unlock();
}
