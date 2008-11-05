/*
 * Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
 *
 * This file is part of fss (Fast SAT Solver).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FSSIO_H
#define FSSIO_H

/**
 * @file fssIO.h
 * @brief I/O module
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup fssIO
 */

#include <string>
#include <iostream>

namespace FastSatSolver {

  /**
   * @brief Common-usage exception containing error message inside.
   * @ingroup fssIO
   */
  class GenericException {
    public:
      /**
       * @param text Text describing exception.
       */
      GenericException(std::string text);
      
      /**
       * @return Returns text describing exception.
       */
      std::string getText();

    private:
      std::string text_;
  };

  /**
   * @brief Sett of common stuff to work with streams.
   * (colored console output, etc.)
   * @ingroup fssIO
   */
  namespace StreamDecorator {
    /**
     * @brief High-level stream manipulator for fixed floats.
     * @ingroup fssIO
     */
    class FixedFloat {
      public:
        /**
         * @param integral length of integral part of float
         * @param decimal length of decimal part of float
         */
        FixedFloat(int integral, int decimal);
        ~FixedFloat();
      private:
        struct Private;
        Private *d;
        friend std::ostream& operator<< (std::ostream &stream, const FixedFloat &);
    };
    /**
     * @brief This behaves as standard stream manipulators.
     */
    std::ostream& operator<< (std::ostream &stream, const FixedFloat &);

    /**
     * @brief Console ouput colors enumeration. Zero means default color.
     * @ingroup fssIO
     */
    enum EColor {
      C_NO_COLOR = 0,
      C_BLUE,
      C_GREEN,
      C_CYAN,
      C_RED,
      C_PURPLE,
      C_BROWN,
      C_LIGHT_GRAY,
      C_DARK_GRAY,
      C_LIGHT_BLUE,
      C_LIGHT_GREEN,
      C_LIGHT_CYAN,
      C_LIGHT_RED,
      C_LIGHT_PURPLE,
      C_YELLOW,
      C_WHITE
    };
    
    /**
     * @attention Global variable is used inside this class.
     * @brief Console output color manipulator
     * @ingroup fssIO
     */
    class Color {
      public:
        /**
         * @param color Desired color of console output. If omitted, default
         * color is assumed.
         */
        Color(EColor color = C_NO_COLOR);
        Color(const Color &);
        ~Color();
        /**
         * @attention Global variable is used inside this class.
         * @brief Enable/disable color ouput @b glaobally.
         */
        static void enable(bool);
        /**
         * @return Returns true if color output is @b gloablly enabled.
         */
        static bool isEnabled();
      private:
        Color& operator= (const Color &);
        static bool useColors;
        struct Private;
        Private *d;
        friend std::ostream& operator<< (std::ostream &, const Color &);
    };
    /**
     * @brief This behaves as standard stream manipulators.
     */
    std::ostream& operator<< (std::ostream &, const Color &);
  } // namespace StreamDecorator

  /**
   * This function uses color output if color output is enabled.
   * @brief Common routine for printing errors.
   * @ingroup fssIO
   */
  inline void printError(std::string szMsg) {
    using namespace StreamDecorator;
    std::cerr << Color(C_RED) << "!!! " << Color() << szMsg << std::endl;
  }

} // namespace FastSatSolver

#endif // FSS_H
