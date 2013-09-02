/// @file soma.cc
/// @brief soma leap interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-08-25

#include "soma.h"

using namespace std;
using namespace soma;
using namespace Leap;
const string usage = "usage: soma";

class test_listener : public Listener
{
    private:
    bool done;
    frame_counter frc;
    finger_counter fic;
    public:
    test_listener (uint64_t window_duration)
        : done (false)
        , fic (200000)
    {
    }
    ~test_listener ()
    {
        clog << frc.fps () << "fps" << endl;
    }
    bool is_done () const
    {
        return done;
    }
    virtual void onInit (const Controller&)
    {
        clog << "onInit()" << endl;
    }
    virtual void onConnect (const Controller&)
    {
        clog << "onConnect()" << endl;
    }
    virtual void onDisconnect (const Controller&)
    {
        clog << "onDisconnect()" << endl;
    }
    virtual void onFrame(const Controller& c)
    {
        const Frame &f = c.frame ();
        frc.update (f.timestamp ());
        fic.update (f.timestamp (), f.pointables ());
        if (fic.is_changed ())
            clog << " fingers " << fic.count () << endl;
        if (fic.count () == 5)
            done = true;
    }
};

int main (int argc, char **argv)
{
    try
    {
        if (argc != 1)
            throw runtime_error (usage);

        test_listener l (200000);
        Controller c (l);

        // set to receive frames in the background
        c.setPolicyFlags (Controller::POLICY_BACKGROUND_FRAMES);

        clog << "press control-C to quit" << endl;

        while (!l.is_done ())
        {
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
