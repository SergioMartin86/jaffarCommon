#pragma once

/**
 * @file logger.hpp
 * @brief Contains common functions related to output and logging
 */

#include <cstdarg>
#include <stdexcept>
#include <cstdio>
#include <unistd.h>
#include "string.hpp"

#ifdef NCURSES
#include <ncurses.h>
#endif

namespace jaffarCommon
{

namespace logger
{

// If we use NCurses, define the following useful functions
#ifdef NCURSES

#define LOG jaffarCommon::_ncursesLog

static bool _useNCurses = false;

template<typename... Args> void _ncursesLog(const char * f, Args... args)
{
  auto string = jaffarCommon::formatString(f, args...);
  if (_useNCurses == true)  printw("%s", string.c_str());
  if (_useNCurses == false) printf("%s", string.c_str());
}

// Function to check for keypress taken from https://github.com/ajpaulson/learning-ncurses/blob/master/kbhit.c
__INLINE__ int kbhit()
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

__INLINE__ int waitForKeyPress()
{
  if (_useNCurses == false)  return getchar(); 

  while (!kbhit())
  {
    usleep(100000ul);
    refresh();
  }
  return getch();
}

__INLINE__ int getKeyPress()
{
  if (_useNCurses == false) return 0; 

  nodelay(stdscr, TRUE);
  noecho();
    
  int ch = getch();

  // restore block and echo
  echo();
  nodelay(stdscr, FALSE);

  return ch;
}

__INLINE__ void initializeTerminal()
{
  // Instructing the log function to use printw
  _useNCurses = true;
  
  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
}

__INLINE__ void clearTerminal()
{
 if (_useNCurses == true) clear();
}

__INLINE__ void finalizeTerminal()
{
  // Instructing the log function to use printf
  _useNCurses = false;

  endwin();
}

__INLINE__ void refreshTerminal()
{
  if (_useNCurses == true) refresh();
}

#else

#define LOG printf
__INLINE__ int waitForKeyPress() { return getchar(); }
__INLINE__ int getKeyPress() { return 0; };
__INLINE__ void initializeTerminal(){}
__INLINE__ void clearTerminal(){}
__INLINE__ void finalizeTerminal(){}
__INLINE__ void refreshTerminal(){}

#endif // NCURSES


} // namespace logger

} // namespace jaffarCommon