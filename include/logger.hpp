#pragma once

/**
 * @file logger.hpp
 * @brief Contains common functions related to output and logging
 */

#include <cstdarg>
#include <stdexcept>
#include <cstdio>
#include <unistd.h>

#ifdef NCURSES
#include <ncurses.h>
#define LOG printw
#else
#define LOG printf
#endif

namespace jaffarCommon
{

// If we use NCurses, define the following useful functions
#ifdef NCURSES

// Function to check for keypress taken from https://github.com/ajpaulson/learning-ncurses/blob/master/kbhit.c
inline int kbhit()
{
  int ch, r;

  // turn off getch() blocking and echo
  nodelay(stdscr, TRUE);
  noecho();

  // check for input
  ch = getch();
  if (ch == ERR) // no input
    r = FALSE;
  else // input
  {
    r = TRUE;
    ungetch(ch);
  }

  // restore block and echo
  echo();
  nodelay(stdscr, FALSE);

  return (r);
}

inline int waitForKeyPress()
{
  while (!kbhit())
  {
    usleep(100000ul);
    refresh();
  }
  return getch();
}

inline int getKeyPress()
{
  nodelay(stdscr, TRUE);
  noecho();
    
  int ch = getch();

  // restore block and echo
  echo();
  nodelay(stdscr, FALSE);

  return ch;
}

inline void initializeTerminal()
{
  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
}

inline void clearTerminal()
{
  clear();
}

inline void finalizeTerminal()
{
  endwin();
}

inline void refreshTerminal()
{
  refresh();
}

#else

inline int waitForKeyPress() { return getchar(); }
inline int getKeyPress() { return 0; };
inline void initializeTerminal(){}
inline void clearTerminal(){}
inline void finalizeTerminal(){}
inline void refreshTerminal(){}

#endif // NCURSES

  
#ifdef EXIT_WITH_ERROR
#undef EXIT_WITH_ERROR
#endif 

#define EXIT_WITH_ERROR(...) jaffarCommon::exitWithError(__FILE__, __LINE__, __VA_ARGS__)
inline void exitWithError [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret < 0) exit(-1);

  std::string outString = outstr;
  free(outstr);

  char info[1024];

  snprintf(info, sizeof(info) - 1, " + From %s:%d\n", fileName, lineNumber);
  outString += info;

  throw std::runtime_error(outString.c_str());
}

} // namespace jaffarCommon