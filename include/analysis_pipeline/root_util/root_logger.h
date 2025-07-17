#ifndef ROOT_LOGGER_H
#define ROOT_LOGGER_H

class RootLogger {
public:
    static RootLogger& instance();

    RootLogger(const RootLogger&) = delete;
    RootLogger& operator=(const RootLogger&) = delete;

private:
    using ROOTHandlerFunc = void(*)(int, bool, const char*, const char*);
    ROOTHandlerFunc previousHandler_;

    RootLogger();
    ~RootLogger();
};

#endif
