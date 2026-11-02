#include <switch.h>

#include <Application/Application.hpp>
#include <Utils/Logger.hpp>

#include <vector>
#include <string>

void Logger::Format(std::vector<char>* logBuf, const char* format, va_list args){
    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (size <= 0){
        return;
    }

    logBuf->resize(size+1);
    std::vsnprintf(logBuf->data(), logBuf->size(), format, args);

    logBuf->resize(size);
}

void Logger::Write(const std::string& msg, bool isImportant) {
    if ((!Rune::Application::IsEmulator) 
        || (!Rune::Application::IsVerbose && !isImportant)){ // If running on console or not verbose, skip
        return;
    }

    svcOutputDebugString(msg.c_str(), msg.size());
}

void Logger::Writef(const std::string& msg, ...){
    if (!Rune::Application::IsEmulator){
        return;
    }

    const char* format = msg.c_str();
    std::vector<char> logFormat;

    va_list args;
    va_start(args, msg);

    Logger::Format(&logFormat, format, args);
    va_end(args);

    Logger::Write(std::string(logFormat.data(), logFormat.size()));
}

void Logger::Importantf(const std::string& msg, ...){
    if (!Rune::Application::IsEmulator){
        return;
    }

    const char* format = msg.c_str();
    std::vector<char> logFormat;

    va_list args;
    va_start(args, msg);

    Logger::Format(&logFormat, format, args);
    va_end(args);
    Logger::Write(std::string(logFormat.data(), logFormat.size()), true);
}

void Logger::Important(const std::string& msg){
    Logger::Write(Logger::kImportantPrefix+msg, true);
}