#include <Gate.hh>
#include <Counter.hh>

class Race
: public Gate
{
private:
    Counter _inside;	// how many tasks are in the race?
    
public:
    Race(bool opened=false)
        : Gate(opened)
        {}
    
    virtual ~Race(void) {}
    
    virtual void pass_in(void) {
        wait();
        _inside++;
    }

    virtual bool pass_out(void) {
        if (_inside--) {
            if (close()) {
                return true;
            } else {
                //assert(!"Closed closed");
                return false;
            }
        } else {
            return false;
        }
    }

    Counter inside(void) {
        return _inside;
    }
};
