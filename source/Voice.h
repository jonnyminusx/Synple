#pragma once

class Voice
{
  public:
    void reset();

    void noteOn(const int note, const int velocity)
    {
        note_ = note;
        velocity_ = velocity;
    }

    void noteOff(const int note)
    {
        if (note_ == note)
        {
            note_ = 0;
            velocity_ = 0;
        }
    }

  private:
    int note_{0};
    int velocity_{0};
};
