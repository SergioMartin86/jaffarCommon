#pragma once

/**
 * @file loggers/ncurses.hpp
 * @brief Contains common functions related to output and logging using NCurses
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdexcept>
#include <ncurses.h>
#include <unistd.h>
#include "../string.hpp"

namespace jaffarCommon
{

namespace logger
{

/**
 * A global setting to store whether NCurses or the normal terminal should be used
 */
static bool _useNCurses = false;

/**
 * Prints the specified formatted string to the NCurses or normal terminal, as configured
 *
 * @param[in] f The formatted string
 * @param[in] args The arguments to the formatted string
 */
template <typename... Args>
__INLINE__ void log(const char *f, Args... args)
{
  auto string = jaffarCommon::string::formatString(f, args...);
  if (_useNCurses == true) printw("%s", string.c_str());
  if (_useNCurses == false) printf("%s", string.c_str());
}

/**
 * Function to check the keyboard buffer for whether there have been any keypress
 *
 * @note Taken from https://github.com/ajpaulson/learning-ncurses/blob/master/kbhit.c
 * @return True, if a key was hit; False, otherwise
 */
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

/**
 * Stalls execution until a key is pressed.
 * It will only return upon the first key press and will report which key it was
 *
 * @note This function employs active polling so should be used sparringly
 *
 * @return Which key was pressed
 */
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

/**
 * Returns any pending key pressed.
 *
 * @note This function returns immediately
 *
 * @return Which key was pressed, the macro ERR if no key was pressed
 */
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

/**
 * Initializes the NCurses terminal
 */
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

/**
 * Clears the NCurses terminal
 */
__INLINE__ void clearTerminal()
{
  if (_useNCurses == true) clear();
}

/**
 * Finalizes the NCurses terminal
 */
__INLINE__ void finalizeTerminal()
{
  // Instructing the log function to use printf
  _useNCurses = false;

  endwin();
}

/**
 * Refreshes the NCurses terminal. This is necessary after every logging operation to update the screen.
 */
__INLINE__ void refreshTerminal()
{
  if (_useNCurses == true) refresh();
}

} // namespace logger

} // namespace jaffarCommon

#undef ERR
#undef OK