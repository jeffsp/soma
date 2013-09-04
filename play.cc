#include <cstdio>
#include <iostream>
#include <cmath>
#include <stdexcept>

using namespace std;

int main()
{
    try
    {
        /*
        const double R=8000; // sample rate (samples per second)
        const double C=261.625565; // frequency of middle-C (hertz)
        //const double F=R/256; // bytebeat frequency of 1*t due to 8-bit truncation (hertz)
        const double V=127; // a volume constant

        const char *cmd = "aplay";
        const char *flags = "w";
        FILE *fp = popen (cmd, flags);
        if (!fp)
            throw runtime_error ("aplay failed");

        for (int t = 0; t < R / 4; ++t)
        {
            uint8_t temp = (sin(t*2*M_PI/R*C)+1)*V;
            //uint8_t temp = t/F*C; // middle C saw wave (bytebeat style)
            //uint8_t temp = (t*5&t>>7)|(t*3&t>>10); // viznut bytebeat composition
            fputc (temp, fp);
            //std::cout<<temp;
        }
        fputc (EOF, fp);

        pclose (fp);
        */

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
