#include "EightWinds/Backend/Logger.h"

#include <cstdarg>
#include <cstdio>
#include <array>

#include "EightWinds/Reflect/Enum.h"

namespace EWE{
    

    namespace Colors{
        //color codes gist here
        //https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124
        static constexpr std::array<std::string_view, 6> colors{
            "\033[0m",  //reset
            "\033[96m", //blue
            "\033[97m", //white
            "\033[92m", //green
            "\033[93m", //yellow
            "\033[91m", //red
        };
        //static constexpr std::string_view red{"\033[31m]"};
        //static constexpr std::string_view green{"\033[32m]"};
        //static constexpr std::string_view yellow{"\033[33m]"};
        //static constexpr std::string_view reset{"\033[0m]"};
    }

    void InternalPrint(Logger::Level ll, std::string_view format, va_list args){

        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format.data(), args);
        printf("%s%s%s: %s", Colors::colors[static_cast<int>(ll)].data(), Reflect::Enum::ToString(ll).data(), Colors::colors[0].data(), buffer);
        fflush(stdout);
    }

    template<> void Logger::Print<Logger::Level::Sanity>(std::string_view fmt, ...){
        if(Logger::Minimum <= Logger::Sanity){
            va_list args;
            va_start(args, fmt);
            InternalPrint(Level::Sanity, fmt, args);
            va_end(args);
        }
    }
    template<> void Logger::Print<Logger::Level::Normal>(std::string_view fmt, ...){
        if(Logger::Minimum <= Logger::Normal){
            va_list args;
            va_start(args, fmt);
            InternalPrint(Level::Normal, fmt, args);
            va_end(args);
        }
    }
    template<> void Logger::Print<Logger::Level::Debug>(std::string_view fmt, ...){
        if(Logger::Minimum <= Logger::Debug){
            va_list args;
            va_start(args, fmt);
            InternalPrint(Level::Debug, fmt, args);
            va_end(args);
        }
    }
    template<> void Logger::Print<Logger::Level::Warning>(std::string_view fmt, ...){
        if(Logger::Minimum <= Logger::Warning){
            va_list args;
            va_start(args, fmt);
            InternalPrint(Level::Warning, fmt, args);
            va_end(args);
        }
    }
    template<> void Logger::Print<Logger::Level::Error>(std::string_view fmt, ...){
        if(Logger::Minimum <= Logger::Error){
            va_list args;
            va_start(args, fmt);
            InternalPrint(Level::Error, fmt, args);
            va_end(args);
        }
    }
}