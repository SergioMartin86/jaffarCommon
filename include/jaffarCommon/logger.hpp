#pragma once

/**
 * @file logger.hpp
 * @brief Contains common functions related to output and logging
 */

#ifdef NCURSES
  #include "loggers/ncurses.hpp"
#else
  #include "loggers/stdio.hpp"
#endif