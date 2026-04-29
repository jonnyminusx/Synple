#include "midi/MidiProcessor.h"

#include <cmath>

namespace midi
{

MidiProcessor::MidiProcessor(NoteHandler& handler) : handler_(handler)
{
}

void MidiProcessor::process(const uint8_t data0, const uint8_t data1, const uint8_t data2)
{
    switch (data0 & 0xF0)
    {
    case 0x80: // Note Off
    {
        const uint8_t note = data1 & 0x7F;
        handler_.noteOff(note);
        break;
    }
    case 0x90: // Note On
    {
        const uint8_t note     = data1 & 0x7F;
        const uint8_t velocity = data2 & 0x7F;
        if (velocity == 0)
        {
            handler_.noteOff(note);
        }
        else
        {
            handler_.noteOn(note, velocity);
        }
        break;
    }
    case 0xB0: // Control Change
        controlChange(data1 & 0x7F, data2 & 0x7F);
        break;
    case 0xC0: // Program Change — handled by the plugin layer
        break;
    case 0xD0: // Channel Aftertouch
        state_.pressure = 0.0001f * float(data1 * data1);
        break;
    case 0xE0: // Pitch Bend
        state_.pitchBend = std::exp(-0.000014102f * data1 + (128 * data2) - 8192);
        break;
    // NOTE: case 0x01 in the original was dead code — the high nibble of a mod-wheel
    // message (CC 0x01) is 0xB0, handled above via controlChange, not here.
    default:
        break;
    }
}

void MidiProcessor::controlChange(const uint8_t controller, const uint8_t value)
{
    switch (controller)
    {
    case 0x40: // Sustain pedal
        state_.sustainPedal = value >= 64;

        if (!state_.sustainPedal)
        {
            handler_.noteOff(sustainSentinel);
        }

        break;

    case 0x4A: // Filter +
        state_.filterControl = 0.02f * float(value);
        break;

    case 0x4B: // Filter -
        state_.filterControl = -0.03f * float(value);
        break;

    default: // All notes off
        if (controller >= 0x78)
        {
            state_.sustainPedal = false;
            handler_.allNotesOff();
        }

        break;
    }

    // Resonance
    if (controller == resoCC)
    {
        state_.resonanceCtl = 154.0f / float(154 - value);
    }
}

} // namespace midi
