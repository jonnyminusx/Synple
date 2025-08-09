#pragma once

class Envelope
{
  public:
    const float silence{0.0001f};

    void setLevel(const float level)
    {
        level_ = level;
    }

    float nextValue()
    {
        level_ *= 0.9999f;
        return level_;
    }

  private:
    float level_{1.0f};
};
