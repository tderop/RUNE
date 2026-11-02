#include <vector>
#include <string>

namespace Logger {
    inline const std::string kImportantPrefix = "[!! RUNE !!] - ";
    inline const std::string kWritePrefix = "[RUNE] - ";

    /**
     * @brief Format a string using printf
     * @param logBuf output buffer
     * @param format string to format
     * @param ... format parameters
     */
    void Format(std::vector<char>* logBuf, const char* format, va_list args);

    /**
     * @brief Log a string to the console using svcOutputDebugString
     * @param msg Message to log
     */
    void Write(const std::string& msg, bool isImportant=false);

    /**
     * @brief Log a string to the console, but like printf
     * @param msg message to format
     * @param ... format parameters
     */
    void Writef(const std::string& msg, ...);

    /**
     * @brief Log an important message to the console using svcOutputDebugString
     * @note Bypasses Rune::Application::IsVerbose requirement
     * @param msg Message to log
     */
    void Important(const std::string& msg);

    /** 
     * @brief Make an important log, but formatted like printf
     * @param msg message to format
     * @param ... format parameters
     */
    void Importantf(const std::string& msg, ...);

} // namespace Logger