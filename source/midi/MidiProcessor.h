#pragma once

#include "midi/CC.h"
#include "midi/MidiState.h"
#include "midi/NoteHandler.h"

#include <cstdint>

namespace midi
{

class MidiProcessor
{
  public:
    explicit MidiProcessor(NoteHandler& handler);

    void process(uint8_t data0, uint8_t data1, uint8_t data2);

    const MidiState& state() const { return state_; }
    MidiState&       state()       { return state_; }

    CC resoCC { 0x47 };

  private:
    void controlChange(uint8_t controller, uint8_t value);

    MidiState    state_;
    NoteHandler& handler_;
};

} // namespace midi
