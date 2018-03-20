#include "ctrl_handler.h"

fx::event ctrl_handler::event_;

ctrl_handler::ctrl_handler()
{
#if defined(_WIN32)
    SetConsoleCtrlHandler(&ctrl_handler::handler, TRUE);
#else
    sigset_t sig_set;
    sigemptyset(&sig_set);

    struct sigaction sig_act;
    sig_act.sa_mask = sig_set;
    sig_act.sa_flags = 0;

    int signals[] =
    {
        SIGHUP,  // Terminal line hangup
        SIGINT,  // Interrupt program
        SIGQUIT, // Quit program
        SIGTSTP, // Stop signal generated from keyboard
        SIGSEGV, // Segmentation violation
        SIGPIPE, // Write on a pipe with no reader
        SIGTERM, // Software termination signal
        SIGUSR1, // User defined signal 1
        SIGUSR2  // User defined signal 2
    };

    for (size_t i = 0; i < (sizeof(signals) / sizeof(signals[0])); i++)
    {
        sig_act.sa_handler = &ctrl_handler::handler;
        sigaction(signals[i], &sig_act, NULL);
    }
#endif
}

#if defined(_WIN32)
BOOL ctrl_handler::handler(DWORD ctrl_type)
{
    switch (ctrl_type)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        event_.signal();
        return TRUE;

    default:
        return FALSE;
    }
}
#else
void ctrl_handler::handler(int sig)
{
    switch (sig)
    {
    case SIGINT:  // interrupt program
    case SIGQUIT: // quit program
    case SIGTSTP: // stop signal generated from keyboard
    case SIGTERM: // software termination signal
    case SIGKILL: // kill program
        event_.signal(); // terminate
        break;

        //case SIGSEGV: // segmentation violation
    case SIGHUP:  // terminal line hangup
    case SIGPIPE: // write on a pipe with no reader
    case SIGUSR1: // User defined signal 1
    case SIGUSR2: // User defined signal 2
        break;

    default:
        break;
    }
}
#endif
