#pragma once

#include <string_view>


namespace EWE{

    struct Logger{
        enum Level{
            None, //dont print anything directly to this
            Sanity, //lowest level, it's expected to work
            Normal, //for stuff like "device was created"
            Debug,
            Warning, //expected to easily break
            Error, //something already broke
        };

        inline static Level Minimum = Level::Sanity;

        template<Level ll = Level::Debug>
        static void Print(std::string_view fmt, ...);
    };
    
    template<> void Logger::Print<Logger::Level::Sanity>(std::string_view fmt, ...);
    template<> void Logger::Print<Logger::Level::Normal>(std::string_view fmt, ...);
    template<> void Logger::Print<Logger::Level::Debug>(std::string_view fmt, ...);
    template<> void Logger::Print<Logger::Level::Warning>(std::string_view fmt, ...);
    template<> void Logger::Print<Logger::Level::Error>(std::string_view fmt, ...);

} //namespace EWE


