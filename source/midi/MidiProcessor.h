#pragma once

#include "midi/CC.h"
#include "midi/MidiState.h"

#include <cstdint>

namespace midi
{

struct NoteHandler;

class MidiProcessor
{
  public:
    explicit MidiProcessor(NoteHandler& handler);

    void process(uint8_t data0, uint8_t data1, uint8_t data2);

    const MidiState& state() const { return state_; }
    MidiState& state() { return state_; }

    void setResoCC(CC cc) { resoCC_ = cc; }
    CC resoCC() const { return resoCC_; }

  private:
    void controlChange(uint8_t controller, uint8_t value);

    MidiState state_;
    NoteHandler& handler_;
    CC resoCC_{0x47};
};

} // namespace midi
