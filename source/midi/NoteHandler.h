#pragma once

namespace midi
{

inline constexpr int sustainSentinel = -1;

struct NoteHandler
{
    virtual ~NoteHandler() = default;
    virtual void noteOn(int note, int velocity) = 0;
    virtual void noteOff(int note) = 0;
    virtual void allNotesOff() = 0;
};

} // namespace midi
