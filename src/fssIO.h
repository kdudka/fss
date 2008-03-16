#ifndef FSSIO_H
#define FSSIO_H

#include <string>
#include <iostream>

namespace FastSatSolver {

  class GenericException {
    public:
      GenericException(std::string text);
      std::string getText();

    private:
      std::string text_;
  };

  namespace StreamDecorator {
    class FixedFloat {
      public:
        FixedFloat(int integral, int decimal);
        ~FixedFloat();
      private:
        struct Private;
        Private *d;
        friend std::ostream& operator<< (std::ostream &stream, const FixedFloat &);
    };
    std::ostream& operator<< (std::ostream &stream, const FixedFloat &);

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
    // ATTENTION: Global variable is used!!
    class Color {
      public:
        Color(EColor color = C_NO_COLOR);
        Color(const Color &);
        ~Color();
        static void enable(bool);
        static bool isEnabled();
      private:
        Color& operator= (const Color &);
        static bool useColors;
        struct Private;
        Private *d;
        friend std::ostream& operator<< (std::ostream &, const Color &);
    };
    std::ostream& operator<< (std::ostream &, const Color &);
  } // namespace StreamDecorator

  inline void printError(std::string szMsg) {
    using namespace StreamDecorator;
    std::cerr << Color(C_RED) << "!!! " << Color() << szMsg << std::endl;
  }

} // namespace FastSatSolver

#endif // FSS_H
