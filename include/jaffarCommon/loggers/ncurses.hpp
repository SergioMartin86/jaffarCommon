#pragma once

/**
 * @file loggers/ncurses.hpp
 * @brief Contains common functions related to output and logging using NCurses
 */

#include "../string.hpp"
#include <cstdarg>
#include <cstdio>
#include <ncurses.h>
#include <stdexcept>
#include <unistd.h>

namespace jaffarCommon
{

namespace logger
{

static bool _useNCurses = false;

template <typename... Args>
__INLINE__ void log(const char *f, Args... args)
{
  auto string = jaffarCommon::string::formatString(f, args...);
  if (_useNCurses == true) printw("%s", string.c_str());
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
  if (_useNCurses == false) return getchar();

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

} // namespace logger

} // namespace jaffarCommon